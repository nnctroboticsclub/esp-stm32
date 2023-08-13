#pragma once

#include <memory>
#include <vector>
#include <stm32/raw_driver/impl/spi.hpp>

namespace stm32::raw_driver::impl {
SPI::SPI(std::shared_ptr<connection::data_link::SPIDevice> device)
    : device(device) {}

void SPI::ACK(TickType_t timeout = portMAX_DELAY) {
  auto trace_ = this->device->IsTraceEnabled();
  this->device->SetTraceEnabled(false);
  int fail_count = 0;

  this->device->SendChar(0x5A);
  this->device->RecvChar();

  while (true) {
    this->device->SendChar(0x00);
    if (auto ch = this->device->RecvChar(); ch == 0x79) {
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
        throw NoData();
      }
    }
  }
  this->device->SendChar(0x79);
  this->device->RecvChar();
  this->device->SetTraceEnabled(trace_);
}

void SPI::Send(const OutboundData &data) {
  using enum OutboundData::SizeMode;
  using enum OutboundData::ChecksumMode;

  switch (data.size) {
    case kU8:
      assert(data.data.size() <= 0x100);
      this->device->SendChar(uint8_t(data.data.size() - 1));
      this->device->RecvChar();
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->device->SendU16((uint16_t)data.data.size());
      this->device->RecvChar();
      this->device->RecvChar();
      break;

    default:  // includes kNone
      break;
  }

  this->device->Send(data.data);

  std::vector<uint8_t> dummy(data.data.size());
  this->device->Recv(dummy);

  switch (data.checksum) {
    case kUnused:
      break;
    case kWithLength:
    case kData:
      uint8_t checksum = data.CalculateChecksum();
      this->device->SendChar(checksum);
      this->device->RecvChar();
      break;
  }

  this->ACK();
}
void SPI::Recv(InboundData &data) {
  std::ranges::fill(data.data, 0x00);

  if (!data.resume) {
    this->device->SendChar(0x00);
    this->device->RecvChar();
  }

  this->device->Send(data.data);
  this->device->Recv(data.data);
}

void SPI::CommandHeader(uint8_t command) {
  std::vector<uint8_t> buf{0x5A, command, (uint8_t)(command ^ 0xff)};

  this->device->Send(buf);
  this->device->Recv(buf);

  this->ACK();
}

void SPI::Sync() {
  ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  this->device->SendChar(0x5A);

  if (auto ret = this->device->RecvChar(); ret != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", ret, 0xA5);
    throw ACKFailed();
  }
  this->ACK();

  ESP_LOGI(TAG, "Connection established");
}

}  // namespace stm32::raw_driver::impl
