#pragma once

#include <gpio_cxx.hpp>

namespace connection::session {

class STM32BL {
 private:
  idf::GPIO_Output reset;
  idf::GPIO_Output boot0;

 public:
  STM32BL(idf::GPIONum reset, idf::GPIONum boot0);

  void TurnOnBoot1();
  void TurnOffBoot1();

  void Reset();
};

}  // namespace connection::session