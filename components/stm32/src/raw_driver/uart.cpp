#include <stm32/raw_driver/impl/uart.hpp>

#include <memory>
#include <vector>

namespace stm32::raw_driver::impl {
UART::UART(std::shared_ptr<connection::data_link::UART> device)
    : device(device) {}

UART::~UART() = default;

void UART::ACK(TickType_t timeout) {
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

void UART::Send(OutboundData const& data) {
  using enum OutboundData::SizeMode;
  using enum OutboundData::ChecksumMode;

  switch (data.size) {
    case kNone:
      break;
    case kU8:
      assert(data.data.size() <= 0x100);
      this->device->SendChar((uint8_t)data.data.size() - 1);
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->device->SendU16((uint16_t)data.data.size());
      break;
  }

  this->device->Send(data.data);

  switch (data.checksum) {
    case kUnused:
      break;
    case kWithLength:
    case kData:
      uint8_t checksum = data.CalculateChecksum();
      this->device->SendChar(checksum);
      break;
  }

  if (data.no_ack) {
    ESP_LOGI(TAG, "No ACK");
  } else {
    this->ACK();
  }
}

std::vector<uint8_t> UART::Recv(size_t length, bool resume) {
  std::vector<uint8_t> result(length, 0x00);

  this->device->RecvExactly(result);

  return result;
}

void UART::CommandHeader(uint8_t command) {
  std::vector<uint8_t> buf{command, uint8_t(command ^ 0xff)};
  this->device->Send(buf);

  this->ACK();
}

void UART::Sync() {
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

}  // namespace stm32::raw_driver::impl
