#include "stm32bl/stm32bl_uart.hpp"

#include <esp_log.h>
#include <memory>

namespace connection::application::stm32bl {

void Stm32BootLoaderUart::SendU16(uint16_t value, bool with_checksum) {
  this->device.SendU16(value);
  if (with_checksum) {
    this->device.SendChar((value >> 8) ^ (value & 0xff));
  }
}

//* Overrided functions
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

void Stm32BootLoaderUart::SendAddress(uint32_t address) {
  this->device.SendU32(address);

  uint32_t checksum = address;
  checksum = (checksum >> 16) ^ (checksum & 0xFFFF);
  checksum = (checksum >> 8) ^ (checksum & 0xFF);
  this->device.SendChar(checksum);

  this->RecvACK();
}

void Stm32BootLoaderUart::SendFlashPage(SpecialFlashPage page) {
  this->SendU16(static_cast<uint16_t>(page), true);
  this->RecvACK(portMAX_DELAY);
}
void Stm32BootLoaderUart::SendFlashPage(std::vector<FlashPage>& pages) {
  std::vector<uint8_t> buf(pages.size() * 2 + 2);

  buf[0] = (pages.size() - 1) >> 8;
  buf[1] = (pages.size() - 1) & 0xff;
  for (int i = 0; i < pages.size(); i++) {
    buf[2 + i * 2] = (pages[i] >> 8) & 0xff;
    buf[2 + i * 2 + 1] = pages[i] & 0xff;
  }

  this->device.Send(buf);
  this->device.SendChar(CalculateChecksum(buf));

  this->RecvACK(portMAX_DELAY);
}

void Stm32BootLoaderUart::SendData(OutboundData& data) {
  using enum OutboundData::SizeMode;

  switch (data.size) {
    case kNone:
      break;
    case kU8:
      assert(data.data.size() <= 0x100);
      this->device.SendChar((uint8_t)data.data.size() - 1);
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->SendU16((uint16_t)data.data.size(), false);
      break;
  }

  this->device.Send(data.data);

  if (data.with_checksum) {
    this->device.SendChar(CalculateChecksum(data.data));
  }
}

// * For Electrical controls

// Todo: This is maybe common function
void Stm32BootLoaderUart::Connect() {
  this->Sync();
  this->Get();
  this->GetVersion();

  return;
}

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

void Stm32BootLoaderUart::SendDataWithChecksum(std::vector<uint8_t>& data) {
  auto n = (uint8_t)(data.size() - 1);
  auto checksum = CalculateChecksum(data) ^ n;

  this->device.SendChar(n);
  this->device.Send(data);
  this->device.SendChar(checksum);
  this->RecvACK();
}

}  // namespace connection::application::stm32bl