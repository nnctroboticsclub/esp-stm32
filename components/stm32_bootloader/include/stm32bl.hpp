#pragma once

#include <vector>

#include "stm32bl/helper.hpp"
#include <gpio_cxx.hpp>

namespace connection::application {
namespace stm32bl {
struct Commands {
  uint8_t get = 0x00;
  uint8_t get_version = 0x01;
  uint8_t get_id = 0x02;
  uint8_t read_memory = 0x11;
  uint8_t go = 0x21;
  uint8_t write_memory = 0x31;
  uint8_t erase = 0x43;
  uint8_t special = 0x50;
  uint8_t extended_special = 0x51;
  uint8_t write_protect = 0x63;
  uint8_t write_unprotect = 0x73;
  uint8_t readout_protect = 0x82;
  uint8_t readout_unprotect = 0x92;
  uint8_t get_checksum = 0xA1;
};

class STM32BootLoader {
  static constexpr const char *TAG = "STM32 BootLoader";

  idf::GPIO_Output reset;
  idf::GPIO_Output boot0;

 public:
  STM32BootLoader(idf::GPIONum reset, idf::GPIONum boot0);
  virtual ~STM32BootLoader();

  void BootBootLoader();

  virtual void Connect() = 0;

  virtual void WriteMemoryBlock(uint32_t address,
                                std::vector<uint8_t> &buf) = 0;
  virtual void Erase(SpecialFlashPage page) = 0;
  virtual void Erase(std::vector<FlashPage> &pages) = 0;
  virtual void Go(uint32_t address) = 0;

  // Helper functions
  void Erase(uint32_t address, uint32_t length);
  void Erase(Pages pages);
  void WriteMemory(uint32_t address, std::vector<uint8_t> &buf);
};
}  // namespace stm32bl

using STM32BootLoader = stm32bl::STM32BootLoader;
}  // namespace connection::application