#include <stm32bl/stm32bl_spi.hpp>

#include <memory.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace stm32bl {
TaskResult Stm32BootLoaderSPI::WaitACKFrame() {
  int fail_count = 0;
  std::vector<uint8_t> buf;

  uint8_t buf1 = 0x00;

  buf = {0x5A};
  this->device.transfer(buf).wait();

  while (1) {
    buf = {0x00};
    this->device.transfer(buf).wait();

    if (buf[0] == 0x79) {
      break;
    } else if (buf[0] == 0x1f) {
      ESP_LOGW(TAG, "NACK");
      return ESP_ERR_INVALID_STATE;
    } else {
      fail_count++;
      if (fail_count % 100 == 0) {
        // ESP_LOGW(TAG, "STM32 SPI ACK Fails %d times (wait 0.5 seconds)",
        //          fail_count);
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      if (fail_count % 100000 == 0) {
        return ESP_ERR_INVALID_STATE;
      }
    }
  }
  buf = {0x79};
  this->device.transfer(buf).wait();
  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Synchronization() {
  ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  buf = {0x5A};
  device.transfer(buf).wait();
  if (buf[0] != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", buf[0],
             0xA5);
    return ESP_ERR_INVALID_STATE;
  }
  this->WaitACKFrame();

  ESP_LOGI(TAG, "Connection established");

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::CommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{0x5A, cmd, (uint8_t)(cmd ^ 0xff)};

  this->device.transfer(buf).wait();
  if (buf[2] != 0x79) {
    // ESP_LOGW(TAG, "Command Header - buf[2] %#02x != 0x79", buf[2]);
  }

  RUN_TASK_V(this->WaitACKFrame());

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::ReadData(uint8_t* buf, size_t size) {
  std::vector<uint8_t> buf1(size, 0xee);
  std::vector<uint8_t> dummy = {0x00};

  this->device.transfer(dummy).wait();

  this->device.transfer(buf1).wait();

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::ReadDataWithoutHeader(uint8_t* buf,
                                                     size_t size) {
  std::vector<uint8_t> dummy = {0x00};

  this->device.transfer(dummy).wait();

  return TaskResult::Ok();
}

Stm32BootLoaderSPI::Stm32BootLoaderSPI(gpio_num_t reset, gpio_num_t boot0,
                                       idf::SPINum spi_host, idf::CS cs)
    : STM32BootLoader(reset, boot0), device(spi_host, cs) {}

Stm32BootLoaderSPI::~Stm32BootLoaderSPI() {}

TaskResult Stm32BootLoaderSPI::Connect() {
  ESP_LOGI(TAG, "Connect...");

  TaskResult ret = ESP_ERR_INVALID_STATE;
  ret = this->Synchronization();
  while (ret.IsErr()) {
    this->BootBootLoader();
    ret = this->Synchronization();
  }

  this->Get();

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Get() {
  RUN_TASK_V(this->CommandHeader(0x00));

  uint8_t buf[0x10]{};
  RUN_TASK_V(this->ReadData(buf, 2));
  RUN_TASK_V(this->ReadDataWithoutHeader(buf + 2, buf[0]));

  RUN_TASK_V(this->WaitACKFrame());

  ESP_LOGI(TAG, "Bootloader version: %d.%d", buf[1] >> 4, buf[1] & 0x0f);

  this->commands.get = buf[2];
  this->commands.get_version = buf[3];
  this->commands.get_id = buf[4];
  this->commands.read_memory = buf[5];
  this->commands.go = buf[6];
  this->commands.write_memory = buf[7];
  this->commands.erase = buf[8];
  this->commands.write_protect = buf[9];
  this->commands.write_unprotect = buf[10];
  this->commands.readout_protect = buf[11];
  this->commands.readout_unprotect = buf[12];
  if (buf[0] > 0x0c) {
    this->commands.get_checksum = buf[13];
  }
  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Erase(SpecialFlashPage page) {
  ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
  RUN_TASK_V(this->CommandHeader(this->commands.erase));

  std::vector<uint8_t> buf{(uint8_t)(page >> 8), (uint8_t)(page & 0xff), 0};
  buf[2] = buf[0] ^ buf[1];

  this->device.transfer(buf).wait();
  RUN_TASK_V(this->WaitACKFrame());

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Erase(std::vector<FlashPage> pages) {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
           0x0800'0000 + pages[pages.size() - 1] * 0x800);
  RUN_TASK_V(this->CommandHeader(this->commands.erase));

  // Send a pages
  {
    std::vector<uint8_t> buf{};
    buf[0] = pages.size() >> 8;
    buf[1] = pages.size() & 0xff;
    buf[2] = CalculateChecksum(buf);
    this->device.transfer(buf).wait();
    RUN_TASK_V(this->WaitACKFrame());
  }

  // Send pages and checksum
  {
    // auto buf = new uint8_t[pages.size() * 2];
    std::vector<uint8_t> buf(pages.size() * 2, 0);
    for (size_t i = 0; i < pages.size(); i++) {
      buf[2 * i] = pages[i] >> 8;
      buf[2 * i + 1] = pages[i] & 0xff;
    }
    buf.push_back(CalculateChecksum(buf));

    this->device.transfer(buf).wait();
    RUN_TASK_V(this->WaitACKFrame());
  }

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Erase(uint32_t addr, uint32_t size) {
  auto pages = MemoryRangeToPages(addr, size);
  if (pages.bank1 && pages.bank2) {
    RUN_TASK_V(this->Erase(SpecialFlashPage::kGlobal));
  } else if (pages.bank1) {
    RUN_TASK_V(this->Erase(SpecialFlashPage::kBank1));
  } else if (pages.bank2) {
    RUN_TASK_V(this->Erase(SpecialFlashPage::kBank2));
  }

  if (pages.pages.size() > 0) {
    RUN_TASK_V(this->Erase(pages.pages));
  }
  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::WriteMemoryBlock(uint32_t addr,
                                                std::vector<uint8_t> buffer) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", addr, buffer.size());
  RUN_TASK_V(this->CommandHeader(this->commands.write_memory));

  {
    std::vector<uint8_t> buf{(uint8_t)(addr >> 24), (uint8_t)(addr >> 16),
                             (uint8_t)(addr >> 8), (uint8_t)(addr & 0xff)};
    buf[4] = CalculateChecksum(buf);
    this->device.transfer(buf).wait();

    RUN_TASK_V(this->WaitACKFrame());
  }

  {
    std::vector<uint8_t> buf{(uint8_t)(buffer.size() - 1)};

    this->device.transfer(buf).wait();
    this->device.transfer(buffer).wait();

    buf[0] = CalculateChecksum(buffer) ^ buf[0];
    this->device.transfer(buf);

    RUN_TASK_V(this->WaitACKFrame());
  }

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::WriteMemory(uint32_t addr,
                                           std::vector<uint8_t> buffer) {
  int remains = buffer.size();
  uint8_t* ptr = buffer.data();
  std::vector<uint8_t> buf(256, 0);
  int offset = 0;
  while (remains > 0) {
    memcpy(buf.data(), ptr, remains > 256 ? 256 : remains);
    RUN_TASK_V(this->WriteMemoryBlock(addr + offset, buf));
    remains = remains - 256 > 0 ? remains - 256 : 0;
    ptr += 256;
    offset += 256;
  }
  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Go(uint32_t addr) {
  ESP_LOGI(TAG, "Go to %08lx", addr);
  RUN_TASK_V(this->CommandHeader(this->commands.go));

  std::vector<uint8_t> buf{(uint8_t)(addr >> 24), (uint8_t)(addr >> 16),
                           (uint8_t)(addr >> 8), (uint8_t)(addr & 0xff)};
  buf[4] = CalculateChecksum(buf);
  this->device.transfer(buf).wait();

  RUN_TASK_V(this->WaitACKFrame());

  return TaskResult::Ok();
}

}  // namespace stm32bl