#pragma once

#include <memory>

#include <gpio_cxx.hpp>
#include "../driver/driver.hpp"
#include "../raw_driver/types/error.hpp"

namespace stm32::session {

class BootLoaderSession;

class Session {
  static constexpr const char *TAG = "[STM32] Session";
  std::shared_ptr<raw_driver::RawDriverBase> raw_bl_driver_;
  idf::GPIO_Output boot0_;
  idf::GPIO_Output reset_;

  friend class BootLoaderSession;

  void SetModeBootLoader();
  void UnsetModeBootLoader();

 public:
  Session(std::shared_ptr<raw_driver::RawDriverBase> raw_bl_driver,
          idf::GPIONum boot0, idf::GPIONum reset);

  void Reset();

  BootLoaderSession EnterBL();
};
}  // namespace stm32::session
