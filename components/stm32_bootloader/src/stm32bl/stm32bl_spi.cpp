#include <stm32bl/stm32bl_spi.hpp>

#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

namespace connection::application::stm32bl {
void Stm32BootLoaderSPI::WaitACKFrame() {
  ESP_LOGI(TAG, "Wait ACK Frame");
  int fail_count = 0;
  std::vector<uint8_t> buf;

  buf = {0x5A};
  auto ret = this->device->transfer(buf).get();

  while (true) {
    buf = {0x00};
    if (auto ch = this->device->transfer(buf).get()[0]; ch == 0x79) {
      break;
    } else if (ch == 0x1f) {
      ESP_LOGW(TAG, "NACK");
      throw ACKFailed();
    } else {
      fail_count++;
      if (fail_count % 100 == 0) {
        // ESP_LOGW(TAG, "STM32 SPI ACK Fails %d times (wait 0.5 seconds)",
        //          fail_count);
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      if (fail_count % 100000 == 0) {
        throw BootLoaderNotBooted();
      }
    }
  }
  buf = {0x79};
  ret = this->device->transfer(buf).get();
  return;
}

void Stm32BootLoaderSPI::Synchronization() {
  ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  buf = {0x5A};

  if (auto ret = this->device->transfer(buf).get(); ret[0] != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", ret[0],
             0xA5);
    throw ACKFailed();
  }
  this->WaitACKFrame();

  ESP_LOGI(TAG, "Connection established");

  return;
}

void Stm32BootLoaderSPI::CommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{0x5A, cmd, (uint8_t)(cmd ^ 0xff)};

  this->device->transfer(buf);

  this->WaitACKFrame();

  return;
}

void Stm32BootLoaderSPI::ReadData(std::vector<uint8_t> &buf) {
  std::vector<uint8_t> dummy = {0x00};
  this->device->transfer(dummy).get();

  this->ReadDataWithoutHeader(buf);
}

void Stm32BootLoaderSPI::ReadDataWithoutHeader(std::vector<uint8_t> &buf) {
  this->device->transfer(buf).get();
}

Stm32BootLoaderSPI::Stm32BootLoaderSPI(idf::GPIONum reset, idf::GPIONum boot0,
                                       idf::SPINum spi_host, idf::CS cs)
    : STM32BootLoader(reset, boot0),
      device(std::make_shared<idf::SPIDevice>(spi_host, cs)) {}

Stm32BootLoaderSPI::~Stm32BootLoaderSPI() = default;

void Stm32BootLoaderSPI::Connect() {
  ESP_LOGI(TAG, "Connect...");

  while (true) {
    this->BootBootLoader();

    try {
      this->Synchronization();
    } catch (ACKFailed &) {
      ESP_LOGE(TAG, "Failed to connect to STM32 Bootloader");
      continue;
    }
    break;
  }

  this->Get();
}

void Stm32BootLoaderSPI::Get() {
  this->CommandHeader(0x00);

  std::vector<uint8_t> buf(2);
  this->ReadData(buf);
  this->WaitACKFrame();
  auto n = buf[0];
  auto version = buf[1];

  ESP_LOGI(TAG, "Bootloader version: %d.%d", version >> 4, version & 0x0f);

  std::vector<uint8_t> commands(n);
  this->ReadDataWithoutHeader(commands);

  this->commands.get = commands[0];
  this->commands.get_version = commands[1];
  this->commands.get_id = commands[2];
  this->commands.read_memory = commands[3];
  this->commands.go = commands[4];
  this->commands.write_memory = commands[5];
  this->commands.erase = commands[6];
  this->commands.write_protect = commands[7];
  this->commands.write_unprotect = commands[8];
  this->commands.readout_protect = commands[9];
  this->commands.readout_unprotect = commands[10];
  if (n > 0x0c) {
    this->commands.get_checksum = buf[13];
  }
  return;
}

void Stm32BootLoaderSPI::Erase(SpecialFlashPage page) {
  ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
  this->CommandHeader(this->commands.erase);

  std::vector<uint8_t> buf;
  buf[0] = ((uint16_t)page >> 8);
  buf[1] = ((uint16_t)page & 0xff);
  buf[2] = buf[0] ^ buf[1];

  this->device->transfer(buf).get();
  this->WaitACKFrame();

  return;
}

void Stm32BootLoaderSPI::Erase(std::vector<FlashPage> &pages) {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
           0x0800'0000 + pages[pages.size() - 1] * 0x800);
  this->CommandHeader(this->commands.erase);

  // Send a pages
  {
    std::vector<uint8_t> buf{};
    buf[0] = pages.size() >> 8;
    buf[1] = pages.size() & 0xff;
    buf[2] = CalculateChecksum(buf);
    this->device->transfer(buf).get();
    this->WaitACKFrame();
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

    this->device->transfer(buf).get();
    this->WaitACKFrame();
  }

  return;
}

void Stm32BootLoaderSPI::WriteMemoryBlock(uint32_t addr,
                                          std::vector<uint8_t> &buffer) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", addr, buffer.size());
  this->CommandHeader(this->commands.write_memory);

  {
    std::vector<uint8_t> buf{(uint8_t)(addr >> 24), (uint8_t)(addr >> 16),
                             (uint8_t)(addr >> 8), (uint8_t)(addr & 0xff)};
    buf[4] = CalculateChecksum(buf);
    this->device->transfer(buf).get();

    this->WaitACKFrame();
  }

  {
    std::vector<uint8_t> buf{(uint8_t)(buffer.size() - 1)};

    this->device->transfer(buf).get();
    this->device->transfer(buffer).get();

    buf[0] = CalculateChecksum(buffer) ^ buf[0];
    this->device->transfer(buf).get();

    this->WaitACKFrame();
  }

  return;
}

void Stm32BootLoaderSPI::Go(uint32_t addr) {
  ESP_LOGI(TAG, "Go to %08lx", addr);
  this->CommandHeader(this->commands.go);

  std::vector<uint8_t> buf{(uint8_t)(addr >> 24), (uint8_t)(addr >> 16),
                           (uint8_t)(addr >> 8), (uint8_t)(addr & 0xff)};
  buf[4] = CalculateChecksum(buf);
  this->device->transfer(buf).get();

  this->WaitACKFrame();

  return;
}

}  // namespace connection::application::stm32bl