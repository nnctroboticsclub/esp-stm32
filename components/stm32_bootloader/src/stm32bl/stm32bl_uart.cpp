#include "stm32bl/stm32bl_uart.hpp"

#include <esp_log.h>
#include <memory>
#include "helper.hpp"

namespace connection::application::stm32bl {

void Stm32BootLoaderUart::SendU16(uint16_t value, bool with_checksum) {
  this->device.SendU16(value);
  if (with_checksum) {
    this->device.SendChar((value >> 8) ^ (value & 0xff));
  }
}

//* Overriden functions
// * For Transferring Datas
void Stm32BootLoaderUart::RecvACK(TickType_t timeout) {
  auto coming_ack = this->device.RecvChar(timeout);

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

void Stm32BootLoaderUart::CommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{cmd, uint8_t(cmd ^ 0xff)};
  this->device.Send(buf);

  this->RecvACK();
}

void Stm32BootLoaderUart::SendData(OutboundData& data) {
  using enum OutboundData::SizeMode;
  using enum OutboundData::ChecksumMode;

  Checksum data_checksum;

  switch (data.size) {
    case kNone:
      break;
    case kU8:
      assert(data.data.size() <= 0x100);
      this->device.SendChar((uint8_t)data.data.size() - 1);
      data_checksum << (uint8_t)(data.data.size() - 1);
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->SendU16((uint16_t)data.data.size(), false);
      data_checksum << (uint16_t)data.data.size();
      break;
  }

  this->device.Send(data.data);

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

      this->device.SendChar(uint8_t(checksum));
      break;
  }

  this->RecvACK();
}

// * For Electrical controls

Stm32BootLoaderUart::Stm32BootLoaderUart(idf::GPIONum reset, idf::GPIONum boot0,
                                         uart_port_t num, int tx, int rx)
    : STM32BootLoader(reset, boot0), device(num) {
  this->device.InstallDriver(tx, rx, 112500, UART_PARITY_EVEN);
  // this->device.SetTraceEnabled(true);
}

Stm32BootLoaderUart::~Stm32BootLoaderUart() = default;

void Stm32BootLoaderUart::Sync() {
  this->device.SendChar(0x7F);

  auto ret = this->device.RecvChar(1000 / portTICK_PERIOD_MS);
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

}  // namespace connection::application::stm32bl