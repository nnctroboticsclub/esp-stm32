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

 public:
  struct Version {
    bool is_valid = false;
    uint8_t major;
    uint8_t minor;
    uint8_t option1;
    uint8_t option2;
  };

 private:
  uint8_t ack = (uint8_t)ACK::ACK;
  bool use_extended_erase = false;

  Version version;
  Commands commands;

  connection::data_link::UART uart;

  void RecvACK(TickType_t timeout = 100 / portTICK_PERIOD_MS);

  void SendWithChecksum(std::vector<uint8_t> &buf);

  void SendU16(uint16_t value, bool with_checksum = false);

  void SendCommandHeader(uint8_t cmd);

  void SendAddress(uint32_t address);

  void DoGetVersion();
  void DoExtendedErase(SpecialFlashPage page);
  void DoExtendedErase(std::vector<uint16_t> &pages);
  void DoErase(SpecialFlashPage page);
  void DoErase(std::vector<uint16_t> &pages);

 public:
  Stm32BootLoaderUart(idf::GPIONum reset, idf::GPIONum boot0, uart_port_t num,
                      int tx, int rx);
  ~Stm32BootLoaderUart() override;

  Version *GetVersion();

  void Sync();
  void Get();

  void Connect() override;

  void WriteMemoryBlock(uint32_t address,
                        std::vector<uint8_t> &buffer) override;

  void Erase(SpecialFlashPage page) override;
  void Erase(std::vector<FlashPage> &pages) override;

  void Go(uint32_t address) override;
};
}  // namespace stm32bl
using STM32BootLoaderUart = stm32bl::Stm32BootLoaderUart;
}  // namespace connection::application