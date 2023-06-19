#include <stm32bl/stm32bl_spi.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace stm32bl {
TaskResult Stm32BootLoaderSPI::WaitACKFrame() {
  int fail_count = 0;
  uint8_t buf1 = 0x00;
  RUN_TASK_V(this->device.Transfer(&buf1, 1))
  while (1) {
    uint8_t buf1 = 0x00;
    RUN_TASK_V(this->device.Transfer(&buf1, 1))

    if (buf1 == 0x79) {
      break;
    } else if (buf1 == 0x1f) {
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
  buf1 = 0x79;
  RUN_TASK_V(device.Transfer(&buf1, 1));
  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Synchronization() {
  ESP_LOGI(TAG, "Sync...");

  uint8_t buf[2] = {0x5A};

  RUN_TASK_V(device.Transfer(buf, 1))
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
  uint8_t buf[] = {0x5A, cmd, (uint8_t)(cmd ^ 0xff)};

  RUN_TASK_V(this->device.Transfer(buf, 3));

  if (buf[2] != 0x79) {
    // ESP_LOGW(TAG, "Command Header - buf[2] %#02x != 0x79", buf[2]);
  }

  RUN_TASK_V(this->WaitACKFrame());

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::ReadData(uint8_t* buf, size_t size) {
  uint8_t dummy = 0;
  RUN_TASK_V(this->device.Transfer(&dummy, 1));

  memset(buf, 0xee, size);

  RUN_TASK_V(this->device.Transfer(buf, size));

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::ReadDataWithoutHeader(uint8_t* buf,
                                                     size_t size) {
  memset(buf, 0x77, size);

  RUN_TASK_V(this->device.Transfer(buf, size));

  return TaskResult::Ok();
}

Stm32BootLoaderSPI::Stm32BootLoaderSPI(gpio_num_t reset, gpio_num_t boot0,
                                       SPIMaster& spi_master, int cs)
    : STM32BootLoader(reset, boot0), device(spi_master.NewDevice(cs)) {}

Stm32BootLoaderSPI::~Stm32BootLoaderSPI() {}

TaskResult Stm32BootLoaderSPI::Connect() {
  ESP_LOGI(TAG, "Connect...");

  TaskResult ret = ESP_ERR_INVALID_STATE;
  while (ret.IsErr()) {
    this->BootBootLoader();
    ret = this->Synchronization();
  }

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

  uint8_t buf[3]{};
  buf[0] = page >> 8;
  buf[1] = page & 0xff;
  buf[2] = buf[0] ^ buf[1];
  RUN_TASK_V(this->device.Transfer(buf, 3));
  RUN_TASK_V(this->WaitACKFrame());

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::Erase(std::vector<FlashPage> pages) {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  RUN_TASK_V(this->CommandHeader(this->commands.erase));

  // Send a pages
  {
    uint8_t buf[3];
    buf[0] = pages.size() >> 8;
    buf[1] = pages.size() & 0xff;
    buf[2] = CalculateChecksum(buf, 2);
    RUN_TASK_V(this->device.Transfer(buf, 3));
    RUN_TASK_V(this->WaitACKFrame());
  }

  // Send pages and checksum
  uint8_t checksum = 0;
  for (auto page : pages) {
    checksum ^= (page >> 8) ^ (page & 0xff);
  }
  RUN_TASK_V(this->device.Transfer((uint8_t*)pages.data(), pages.size() * 2));
  RUN_TASK_V(this->device.Transfer(&checksum, 1));
  RUN_TASK_V(this->WaitACKFrame());

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

TaskResult Stm32BootLoaderSPI::WriteMemoryBlock(uint32_t addr, uint8_t* buffer,
                                                size_t size) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", addr, size);
  RUN_TASK_V(this->CommandHeader(this->commands.write_memory));

  {
    uint8_t buf[5]{};
    buf[0] = addr >> 24;
    buf[1] = addr >> 16;
    buf[2] = addr >> 8;
    buf[3] = addr & 0xff;
    buf[4] = CalculateChecksum(buf, 4);
    RUN_TASK_V(this->device.Transfer(buf, 5));

    RUN_TASK_V(this->WaitACKFrame());
  }

  {
    uint8_t size_byte = size - 1;
    uint8_t checksum = CalculateChecksum(buffer, size) ^ size_byte;

    RUN_TASK_V(this->device.Transfer(&size_byte, 1));
    RUN_TASK_V(this->device.Transfer(buffer, size));
    RUN_TASK_V(this->device.Transfer(&checksum, 1));

    RUN_TASK_V(this->WaitACKFrame());
  }

  return TaskResult::Ok();
}

TaskResult Stm32BootLoaderSPI::WriteMemory(uint32_t addr, uint8_t* buffer,
                                           size_t size) {
  int remains = size;
  uint8_t* ptr = buffer;
  uint8_t buf[256];
  int offset = 0;
  while (remains > 0) {
    memset(buf, 0, 256);
    memcpy(buf, ptr, remains > 256 ? 256 : remains);
    RUN_TASK_V(this->WriteMemoryBlock(addr + offset, buf,
                                      remains > 256 ? 256 : remains));
    remains = remains - 256 > 0 ? remains - 256 : 0;
    ptr += 256;
    offset += 256;
  }
  return size;
}

TaskResult Stm32BootLoaderSPI::Go(uint32_t addr) {
  ESP_LOGI(TAG, "Go to %08lx", addr);
  RUN_TASK_V(this->CommandHeader(this->commands.go));

  uint8_t buf[5]{};
  buf[0] = addr >> 24;
  buf[1] = addr >> 16;
  buf[2] = addr >> 8;
  buf[3] = addr & 0xff;
  buf[4] = CalculateChecksum(buf, 4);
  RUN_TASK_V(this->device.Transfer(buf, 5));

  RUN_TASK_V(this->WaitACKFrame());

  return TaskResult::Ok();
}

}  // namespace stm32bl