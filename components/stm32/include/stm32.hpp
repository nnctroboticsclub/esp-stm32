#pragma once

#include <driver/gpio.h>

#include <memory>

#include "stm32/session/stm32.hpp"
#include "stm32/session/bootloader.hpp"

namespace stm32 {

class STM32 {
  std::shared_ptr<session::STM32> stm32;
  std::shared_ptr<stm32::driver::BLDriver> bl_driver_;

 public:
  STM32(idf::GPIONum boot0, idf::GPIONum reset)
      : stm32(std::make_shared<session::STM32>(boot0, reset)) {}

  STM32(idf::GPIONum boot0, idf::GPIONum reset,
        std::shared_ptr<stm32::driver::BLDriver> bl_driver)
      : STM32(boot0, reset) {
    this->bl_driver_ = bl_driver;
  }

  void SetBootLoader(std::shared_ptr<stm32::driver::BLDriver> bl_driver) {
    this->bl_driver_ = bl_driver;
  }

  void Reset() { stm32->Reset(); }

  std::optional<session::BootLoaderSession> EnterBootloader(int tries = 5) {
    return stm32->TryEnterBL(this->bl_driver_, tries);
  }
};

}  // namespace stm32
