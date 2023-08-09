#pragma once

#include <esp_log.h>

#include <memory>
#include <vector>

#include <connection/data_link/uart.hpp>

#include "../types/error.hpp"
#include "../types/inbounddata.hpp"
#include "../types/outbounddata.hpp"
#include "./checksum.hpp"

namespace stm32::raw_driver::impl {
using UartDataLink = connection::data_link::UART;

class UART {
  static constexpr const char *TAG = "RawDriver<UART>";

  std::shared_ptr<UartDataLink> device;

 public:
  explicit UART(std::shared_ptr<connection::data_link::UART> device)
      : device(device) {}

  void ACK(TickType_t timeout = portMAX_DELAY) {
    auto coming_ack = this->device->RecvChar(timeout);

    if (coming_ack == 0x79) {
      return;
    } else if (coming_ack == 0x1f) {
      ESP_LOGE(TAG, "Received NACK");
      throw ACKFailed();
    } else {
      ESP_LOGE(TAG, "Unknown ACK %#02x", coming_ack);
      throw ACKFailed();
    }
  }

  void Send(const OutboundData &data) {
    using enum OutboundData::SizeMode;
    using enum OutboundData::ChecksumMode;

    Checksum data_checksum;

    switch (data.size) {
      case kNone:
        break;
      case kU8:
        assert(data.data.size() <= 0x100);
        this->device->SendChar((uint8_t)data.data.size() - 1);
        data_checksum << (uint8_t)(data.data.size() - 1);
        break;
      case kU16:
        assert(data.data.size() <= 0xffff);
        this->device->SendU16((uint16_t)data.data.size());
        data_checksum << (uint16_t)data.data.size();
        break;
    }

    this->device->Send(data.data);

    Checksum checksum;
    checksum << data.checksum_base;
    switch (data.checksum) {
      case kUnused:
        break;
      case kWithLength:
        checksum << data_checksum;
        [[fallthrough]];

      case kData:
        checksum << data.data;

        this->device->SendChar(uint8_t(checksum));
        break;
    }

    this->ACK();
  }
  void Recv(InboundData &data) { this->device->RecvExactly(data.data); }

  void CommandHeader(uint8_t command) {
    std::vector<uint8_t> buf{command, uint8_t(command ^ 0xff)};
    this->device->Send(buf);

    this->ACK();
  }

  void Sync() {
    this->device->SendChar(0x7F);

    auto ret = this->device->RecvChar(1000 / portTICK_PERIOD_MS);
    if (ret == 0x79) {
      return;
    } else if (ret == 0x1f) {
      ESP_LOGW(TAG, "NACK");
      throw ACKFailed();
    } else {
      ESP_LOGW(TAG, "Unknown sync byte %#02x", ret);
      throw ACKFailed();
    }
  }
};
}  // namespace stm32::raw_driver::impl
