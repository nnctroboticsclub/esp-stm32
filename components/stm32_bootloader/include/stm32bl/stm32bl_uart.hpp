#pragma once

#include <uart.hpp>
#include <inttypes.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

#include <vector>
#include <uart.hpp>
#include "../stm32bl.hpp"

namespace connection::application {
namespace stm32bl {
class Stm32BootLoaderUart : public STM32BootLoader {
  static constexpr const char *TAG = "STM32 BootLoader[UART]";

 private:
  uint8_t ack = (uint8_t)ACK::ACK;
  bool use_extended_erase = false;

  connection::data_link::UART device;

  inline void ReadData(std::vector<uint8_t> &data) override {
    this->device.RecvExactly(data);
  }
  inline void ReadDataWithoutHeader(std::vector<uint8_t> &data) override {
    this->device.RecvExactly(data);
  }

  void RecvACK(TickType_t timeout = 100 / portTICK_PERIOD_MS) override;

  //! Some functions for transferring datas
  void SendDataWithChecksum(std::vector<uint8_t> &data) override;
  void CommandHeader(uint8_t cmd) override;
  void SendAddress(uint32_t address) override;
  void SendFlashPage(SpecialFlashPage address) override;
  void SendFlashPage(std::vector<FlashPage> &address) override;

  void SendData(OutboundData &data) override;

  void SendU16(uint16_t value, bool with_checksum = false);

  void Erase(SpecialFlashPage page) override;
  void Erase(std::vector<FlashPage> &pages) override;

 public:
  Stm32BootLoaderUart(idf::GPIONum reset, idf::GPIONum boot0, uart_port_t num,
                      int tx, int rx);
  ~Stm32BootLoaderUart() override;

  void Sync();

  void Connect() override;
};
}  // namespace stm32bl
using STM32BootLoaderUart = stm32bl::Stm32BootLoaderUart;
}  // namespace connection::application