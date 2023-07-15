#include "stm32bl/stm32bl_uart.hpp"

#include <esp_log.h>
#include <memory>

namespace connection::application::stm32bl {

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

void Stm32BootLoaderUart::SendWithChecksum(std::vector<uint8_t>& buf) {
  this->device.Send(buf);
  this->device.SendChar(CalculateChecksum(buf));
}

void Stm32BootLoaderUart::SendU16(uint16_t value, bool with_checksum) {
  this->device.SendU16(value);
  if (with_checksum) {
    this->device.SendChar((value >> 8) ^ (value & 0xff));
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

void Stm32BootLoaderUart::DoGetVersion() {
  this->CommandHeader(this->commands.get_version);

  std::vector<uint8_t> buf(3);
  this->device.Recv(buf, 500 / portTICK_PERIOD_MS);
  this->version = Version(buf);
  this->RecvACK();

  return;
}

void Stm32BootLoaderUart::DoExtendedErase(SpecialFlashPage page) {
  this->CommandHeader(this->commands.erase);

  this->SendU16((uint16_t)page, true);
  this->RecvACK(portMAX_DELAY);

  return;
}

void Stm32BootLoaderUart::DoExtendedErase(std::vector<uint16_t>& pages) {
  this->CommandHeader(this->commands.erase);

  std::vector<uint8_t> buf(pages.size() * 2 + 2);

  buf[0] = (pages.size() - 1) >> 8;
  buf[1] = (pages.size() - 1) & 0xff;
  for (int i = 0; i < pages.size(); i++) {
    buf[2 + i * 2] = (pages[i] >> 8) & 0xff;
    buf[2 + i * 2 + 1] = pages[i] & 0xff;
  }

  this->SendWithChecksum(buf);

  this->RecvACK(portMAX_DELAY);

  return;
}

void Stm32BootLoaderUart::DoErase(SpecialFlashPage) {
  // TODO(syoch): Impl
  throw NotImplemented();
}
void Stm32BootLoaderUart::DoErase(std::vector<uint16_t>&) {
  // TODO(syoch): Impl
  throw NotImplemented();
}

Stm32BootLoaderUart::Stm32BootLoaderUart(idf::GPIONum reset, idf::GPIONum boot0,
                                         uart_port_t num, int tx, int rx)
    : STM32BootLoader(reset, boot0), device(num) {
  this->device.InstallDriver(tx, rx, 112500, UART_PARITY_EVEN);
  // this->device.SetTraceEnabled(true);
}

Stm32BootLoaderUart::~Stm32BootLoaderUart() = default;

void Stm32BootLoaderUart::Connect() {
  this->Sync();
  this->Get();
  this->GetVersion();

  ESP_LOGI(TAG, "Boot Loader version = %d.%d", this->GetVersion()->major,
           this->GetVersion()->minor);

  return;
}

Version* Stm32BootLoaderUart::GetVersion() {
  if (!this->version.updated) {
    this->version.updated = true;
    this->GetVersion();
  }

  return &this->version;
}

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

void Stm32BootLoaderUart::Get() {
  this->CommandHeader(this->commands.get);

  std::vector<uint8_t> buf(2);
  this->ReadData(buf);
  this->version.UpdateVersion(buf[1]);

  std::vector<uint8_t> raw_commands(buf[0]);
  this->ReadDataWithoutHeader(raw_commands);

  this->commands = Commands(raw_commands);

  this->RecvACK();

  if (this->commands.erase == 0x44) {
    this->use_extended_erase = true;
  }

  return;
}

void Stm32BootLoaderUart::WriteMemoryBlock(uint32_t address,
                                           std::vector<uint8_t>& buffer) {
  this->CommandHeader(0x31);

  this->SendAddress(address);

  this->device.SendChar((uint8_t)(buffer.size() - 1));

  uint8_t checksum = buffer.size() - 1;
  for (int i = 0; i < buffer.size(); i++) {
    checksum ^= buffer[i];
  }
  this->device.Send(buffer);
  this->device.SendChar(checksum);

  this->RecvACK();

  return;
}

void Stm32BootLoaderUart::Go(uint32_t address) {
  ESP_LOGI(TAG, "Go command");
  this->CommandHeader(this->commands.go);

  this->SendAddress(address);

  return;
}

void Stm32BootLoaderUart::Erase(SpecialFlashPage page) {
  if (this->use_extended_erase) {
    this->DoExtendedErase(page);
  } else {
    this->DoErase(page);
  }
}

void Stm32BootLoaderUart::Erase(std::vector<FlashPage>& pages) {
  if (this->use_extended_erase) {
    this->DoExtendedErase(pages);
  } else {
    this->DoErase(pages);
  }
}
}  // namespace connection::application::stm32bl