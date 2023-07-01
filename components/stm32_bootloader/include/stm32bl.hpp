#pragma once

#include <vector>

#include <result.hpp>
#include <gpio_cxx.hpp>

namespace stm32bl {
class STM32BootLoader {
  static constexpr const char* TAG = "STM32 BootLoader";

  idf::GPIO_Output reset;
  idf::GPIO_Output boot0;

 public:
  STM32BootLoader(idf::GPIONum reset, idf::GPIONum boot0);
  virtual ~STM32BootLoader();

  void BootBootLoader();

  virtual TaskResult Connect() = 0;

  virtual TaskResult WriteMemory(uint32_t address,
                                 std::vector<uint8_t> buf) = 0;
  virtual TaskResult Erase(uint32_t address, uint32_t length) = 0;
  virtual TaskResult Go(uint32_t address) = 0;

  // Helper functions
  TaskResult Erase(uint32_t address, uint32_t length, uint32_t size);
};
}  // namespace stm32bl