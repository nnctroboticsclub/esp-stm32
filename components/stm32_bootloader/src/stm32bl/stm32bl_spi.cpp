#include <stm32bl/stm32bl_spi.hpp>

#include <helper.hpp>

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

void Stm32BootLoaderSPI::Sync() {
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
    : STM32BootLoader(reset, boot0), device(spi_host, cs) {}

Stm32BootLoaderSPI::~Stm32BootLoaderSPI() = default;

void Stm32BootLoaderSPI::SendData(OutboundData &data) {
  using enum OutboundData::SizeMode;
  using enum OutboundData::ChecksumMode;

  Checksum data_checksum;

  switch (data.size) {
    case kU8:
      assert(data.data.size() <= 0x100);
      this->device.SendChar(uint8_t(data.data.size() - 1));
      data_checksum << uint8_t(data.data.size() - 1);
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->device.SendU16((uint16_t)data.data.size());
      data_checksum << (uint16_t)data.data.size();
      break;

    default:  // includes kNone
      break;
  }

  this->device.Send(data.data);

  Checksum checksum;
  switch (data.checksum) {
    case kUnused:
      break;
    case kWithLength:
      checksum << data_checksum;
      [[fallthrough]];
    case kData:
      checksum << data.data;

      this->device.SendChar(uint8_t(checksum));
      break;
  }

  this->RecvACK();
}

}  // namespace connection::application::stm32bl