#pragma once

#include <memory>
#include <optional>

#include <gpio_cxx.hpp>
#include "../driver/driver.hpp"

namespace stm32::session {

class BootLoaderSession;

class Session {
  static constexpr const char *TAG = "[STM32] Session";
  idf::GPIO_Output boot0_;
  idf::GPIO_Output reset_;

  friend class BootLoaderSession;

  void SetModeBootLoader();
  void UnsetModeBootLoader();

 public:
  Session(idf::GPIONum boot0, idf::GPIONum reset);

  void Reset();

  std::optional<BootLoaderSession> TryEnterBL(
      std::shared_ptr<driver::BLDriver> raw_bl_driver, int tries = 5);
};
}  // namespace stm32::session
