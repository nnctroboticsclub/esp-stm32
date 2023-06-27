#pragma once

#include <uart.hpp>
#include <inttypes.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <result.hpp>

#include <vector>

#include "../stm32bl.hpp"

namespace stm32bl {

class Stm32BootLoaderUart : public STM32BootLoader {
  static constexpr const char* TAG = "STM32 BootLoader[UART]";

 public:
  struct Version {
    bool is_valid = false;
    uint8_t major;
    uint8_t minor;
    uint8_t option1;
    uint8_t option2;
  };

 private:
  bool use_extended_erase = false;

  uint8_t get = 0x00;
  uint8_t get_version = 0x01;
  uint8_t get_id = 0x02;
  uint8_t read_memory = 0x11;
  uint8_t go = 0x21;
  uint8_t write_memory = 0x31;
  uint8_t erase = 0x43;
  uint8_t write_protect = 0x63;
  uint8_t write_unprotect = 0x73;
  uint8_t readout_protect = 0x82;
  uint8_t readout_unprotect = 0x92;
  uint8_t ack = 0x79;

  using FlashPage = uint16_t;
  static constexpr const FlashPage global = 0xffff;
  static constexpr const FlashPage bank1 = 0xfffe;
  static constexpr const FlashPage bank2 = 0xfffd;

  Version version;

  UART uart;

  TaskResult RecvACK(TickType_t timeout = 100 / portTICK_PERIOD_MS);

  void SendWithChecksum(uint8_t* buf, size_t size);

  void SendU16(uint8_t high, uint8_t low, bool with_checksum = false);

  void SendU16(uint16_t value, bool with_checksum = false);

  void SendCommand(uint8_t command);

  void SendAddress(uint32_t address);

  TaskResult DoGetVersion();
  TaskResult DoExtendedErase(FlashPage page);
  TaskResult DoExtendedErase(std::vector<uint16_t> pages);
  TaskResult DoErase(FlashPage page);
  TaskResult DoErase(std::vector<uint16_t> pages);

 public:
  Stm32BootLoaderUart(gpio_num_t reset, gpio_num_t boot0, uart_port_t num,
                      int tx, int rx);
  virtual ~Stm32BootLoaderUart();

  Version* GetVersion();

  TaskResult Sync();
  TaskResult Get();

  TaskResult Connect() override;

  TaskResult WriteMemoryBlock(uint32_t address, std::vector<uint8_t> buffer);
  TaskResult WriteMemory(uint32_t address,
                         std::vector<uint8_t> buffer) override;

  TaskResult Erase(FlashPage page);
  TaskResult Erase(std::vector<uint16_t> pages);
  TaskResult BulkErase(std::vector<uint16_t> pages);
  TaskResult Erase(uint32_t address, uint32_t length) override;

  TaskResult Go(uint32_t address) override;
};
}  // namespace stm32bl