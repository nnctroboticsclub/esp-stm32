#pragma once

#include <memory>

#include <driver/gpio.h>

#include "stm32/session/stm32.hpp"
#include "stm32/session/bootloader.hpp"

namespace stm32 {

class STM32 {
  std::shared_ptr<session::STM32> stm32;
  std::shared_ptr<stm32::driver::BLDriver> bl_driver;

 public:
  STM32(idf::GPIONum boot0, idf::GPIONum reset,
        std::shared_ptr<stm32::driver::BLDriver> bl_driver)
      : stm32(std::make_shared<session::STM32>(boot0, reset)),
        bl_driver(bl_driver) {}

  void Reset() const { stm32->Reset(); }

  void EnterBootloader(int tries = 5) const {
    stm32->TryEnterBL(this->bl_driver, tries);
  }
};

}  // namespace stm32
