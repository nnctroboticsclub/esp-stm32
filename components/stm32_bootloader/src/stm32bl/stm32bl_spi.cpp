#include <stm32bl/stm32bl_spi.hpp>

#include <memory>
#include <ranges>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_debug_helpers.h>

namespace connection::application::stm32bl {
void Stm32BootLoaderSPI::RecvACK(TickType_t timeout) {
  auto trace_ = this->device.IsTraceEnabled();
  this->device.SetTraceEnabled(false);
  int fail_count = 0;

  this->device.SendChar(0x5A);
  this->device.RecvChar();

  while (true) {
    this->device.SendChar(0x00);
    if (auto ch = this->device.RecvChar(); ch == 0x79) {
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
  this->device.SendChar(0x79);
  this->device.RecvChar();
  this->device.SetTraceEnabled(trace_);
  return;
}

void Stm32BootLoaderSPI::Synchronization() {
  ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  this->device.SendChar(0x5A);

  if (auto ret = this->device.RecvChar(); ret != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", ret, 0xA5);
    throw ACKFailed();
  }
  this->RecvACK();

  ESP_LOGI(TAG, "Connection established");

  return;
}

void Stm32BootLoaderSPI::CommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{0x5A, cmd, (uint8_t)(cmd ^ 0xff)};
  this->device.Send(buf);

  this->device.Recv(buf);

  this->RecvACK();

  return;
}

void Stm32BootLoaderSPI::ReadData(std::vector<uint8_t> &buf) {
  this->device.SendChar(0x00);
  this->device.RecvChar();

  this->ReadDataWithoutHeader(buf);
}

void Stm32BootLoaderSPI::ReadDataWithoutHeader(std::vector<uint8_t> &buf) {
  std::ranges::fill(buf, 0x00);

  this->device.Send(buf);
  this->device.Recv(buf);
}

Stm32BootLoaderSPI::Stm32BootLoaderSPI(idf::GPIONum reset, idf::GPIONum boot0,
                                       idf::SPIMaster &spi_host, idf::CS cs)
    : STM32BootLoader(reset, boot0), device(spi_host, cs) {
  this->device.SetTraceEnabled(false);
}

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

void Stm32BootLoaderSPI::Erase(SpecialFlashPage page) {
  ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
  this->CommandHeader(this->commands.erase);

  std::vector<uint8_t> buf(3);
  buf[0] = ((uint16_t)page >> 8);
  buf[1] = ((uint16_t)page & 0xff);
  buf[2] = buf[0] ^ buf[1];

  this->device.Send(buf);
  this->device.Recv(buf);
  this->RecvACK();

  return;
}

void Stm32BootLoaderSPI::Erase(std::vector<FlashPage> &pages) {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
           0x0800'0000 + pages[pages.size() - 1] * 0x800);
  this->CommandHeader(this->commands.erase);

  uint8_t checksum = 0;

  // Send a pages
  {
    std::vector<uint8_t> buf(3);
    buf[0] = pages.size() >> 8;
    buf[1] = pages.size() & 0xff;
    checksum ^= CalculateChecksum(buf);
    buf[2] = checksum;

    this->device.Send(buf);
    this->device.Recv(buf);
    this->RecvACK();
  }

  // Send pages and checksum
  {
    std::vector<uint8_t> buf(pages.size() * 2, 0x77);
    for (size_t i = 0; i < pages.size(); i++) {
      buf[2 * i] = pages[i] >> 8;
      buf[2 * i + 1] = pages[i] & 0xff;
    }
    checksum = 0x5a ^ CalculateChecksum(buf);  // WHAT IS 0x5A (NANNMO-WAKARAN)
    buf.push_back(checksum);

    this->device.Send(buf);
    this->device.Recv(buf);
    this->RecvACK();
  }

  return;
}

void Stm32BootLoaderSPI::SendAddress(uint32_t address) {
  uint32_t ch = address;
  ch = (ch >> 16) ^ (ch & 0xffff);
  ch = (ch >> 8) ^ (ch & 0xff);

  this->device.SendU32(address);
  this->device.RecvU32();

  this->device.SendChar((char)ch);
  this->device.RecvChar();

  this->RecvACK();
}

void Stm32BootLoaderSPI::SendDataWithChecksum(std::vector<uint8_t> &data) {
  auto n = (uint8_t)(data.size() - 1);
  auto checksum = CalculateChecksum(data) ^ n;

  this->device.SendChar(n);
  this->device.RecvChar();

  this->device.Send(data);
  this->device.Recv(data);

  this->device.SendChar(checksum);
  this->device.RecvChar();

  this->RecvACK();
}

}  // namespace connection::application::stm32bl