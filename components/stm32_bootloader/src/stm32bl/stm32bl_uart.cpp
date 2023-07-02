#include "stm32bl/stm32bl_uart.hpp"

#include <esp_log.h>
#include <memory>

namespace connection::application::stm32bl {

void Stm32BootLoaderUart::RecvACK(TickType_t timeout) {
  auto coming_ack = this->uart.RecvChar(timeout);

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
  this->uart.Send(buf);
  this->uart.SendChar(CalculateChecksum(buf));
}

void Stm32BootLoaderUart::SendU16(uint16_t value, bool with_checksum) {
  this->uart.SendU16(value);
  if (with_checksum) {
    this->uart.SendChar((value >> 8) ^ (value & 0xff));
  }
}

void Stm32BootLoaderUart::SendCommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{cmd, uint8_t(cmd ^ 0xff)};
  this->uart.Send(buf);
}

void Stm32BootLoaderUart::SendAddress(uint32_t address) {
  this->uart.SendU32(address);

  uint32_t checksum = address;
  checksum = (checksum >> 16) ^ (checksum & 0xFFFF);
  checksum = (checksum >> 8) ^ (checksum & 0xFF);
  this->uart.SendChar(checksum);
}

void Stm32BootLoaderUart::DoGetVersion() {
  this->SendCommandHeader(this->commands.get_version);
  this->RecvACK();

  std::vector<uint8_t> buf(3);
  this->uart.Recv(buf, 500 / portTICK_PERIOD_MS);
  this->version.major = buf[0] >> 4;
  this->version.major = buf[0] >> 4;
  this->version.option1 = buf[1];
  this->version.option1 = buf[2];
  this->RecvACK();

  return;
}

void Stm32BootLoaderUart::DoExtendedErase(SpecialFlashPage page) {
  this->SendCommandHeader(this->commands.erase);
  this->RecvACK();

  this->SendU16((uint16_t)page, true);
  this->RecvACK();

  return;
}

void Stm32BootLoaderUart::DoExtendedErase(std::vector<uint16_t>& pages) {
  this->SendCommandHeader(this->commands.erase);
  this->RecvACK();

  std::vector<uint8_t> buf(pages.size() * 2 + 2);

  buf[0] = (pages.size() - 1) >> 8;
  buf[1] = (pages.size() - 1) & 0xff;
  for (int i = 0; i < pages.size(); i++) {
    buf[2 + i * 2] = (pages[i] >> 8) & 0xff;
    buf[2 + i * 2 + 1] = pages[i] & 0xff;
  }

  this->SendWithChecksum(buf);

  this->RecvACK();

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
    : STM32BootLoader(reset, boot0), uart(num) {
  this->uart.InstallDriver(tx, rx, 112500, UART_PARITY_EVEN);
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

Stm32BootLoaderUart::Version* Stm32BootLoaderUart::GetVersion() {
  if (!this->version.is_valid) {
    this->version.is_valid = true;
    this->GetVersion();
  }

  return &this->version;
}

void Stm32BootLoaderUart::Sync() {
  this->uart.SendChar(0x7F);

  auto ret = this->uart.RecvChar();
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
  this->SendCommandHeader(this->commands.get);
  this->RecvACK();

  this->ack = this->uart.RecvChar();

  auto n = this->uart.RecvChar();

  auto byte = this->uart.RecvChar();
  this->version.major = byte >> 4;
  this->version.minor = byte & 0xf;

  std::vector<uint8_t> buf(n);
  this->uart.Recv(buf, 100 / portTICK_PERIOD_MS);

  this->commands.get = buf[0];
  this->commands.get_version = buf[1];
  this->commands.get_id = buf[2];
  this->commands.read_memory = buf[3];
  this->commands.go = buf[4];
  this->commands.write_memory = buf[5];
  this->commands.erase = buf[6];
  this->commands.write_protect = buf[7];
  this->commands.write_unprotect = buf[8];
  this->commands.readout_protect = buf[9];
  this->commands.readout_unprotect = buf[10];
  // TODO: Add GetChecksum command
  // this->get_checksum = buf[11];

  this->RecvACK();

  if (this->commands.erase == 0x44) {
    this->use_extended_erase = true;
  }

  return;
}

void Stm32BootLoaderUart::WriteMemoryBlock(uint32_t address,
                                           std::vector<uint8_t>& buffer) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", address, buffer.size());
  this->SendCommandHeader(0x31);
  this->RecvACK();

  this->SendAddress(address);
  this->RecvACK();

  this->uart.SendChar((uint8_t)(buffer.size() - 1));

  uint8_t checksum = buffer.size() - 1;
  for (int i = 0; i < buffer.size(); i++) {
    checksum ^= buffer[i];
  }
  this->uart.Send(buffer);
  this->uart.SendChar(checksum);

  this->RecvACK();

  return;
}

void Stm32BootLoaderUart::Go(uint32_t address) {
  ESP_LOGI(TAG, "Go command");
  this->SendCommandHeader(this->commands.go);
  this->RecvACK();

  this->SendAddress(address);
  this->RecvACK();

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