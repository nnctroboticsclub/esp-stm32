#pragma once

#include <result.hpp>
#include <driver/gpio.h>

namespace stm32bl {
class STM32BootLoader {
  static constexpr const char* TAG = "STM32 BootLoader";

 private:
  gpio_num_t reset;
  gpio_num_t boot0;

 public:
  STM32BootLoader(gpio_num_t reset, gpio_num_t boot0);
  virtual ~STM32BootLoader();

  void BootBootLoader();

  virtual TaskResult WriteMemory(uint32_t address, uint8_t* buffer,
                                 size_t size) = 0;
  virtual TaskResult Erase(uint32_t address, uint32_t length) = 0;
  virtual TaskResult Go(uint32_t address) = 0;
};
}  // namespace stm32bl