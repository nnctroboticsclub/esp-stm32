#pragma once

#include <memory>

#include <gpio_cxx.hpp>
#include "../driver/driver.hpp"
#include "../raw_driver/types/error.hpp"

namespace stm32 {

template <typename RawBLDriver>
  requires raw_driver::RawDriverConcept<RawBLDriver>
class Session {
  std::shared_ptr<RawBLDriver> raw_bl_driver_;
  idf::GPIO_Output boot0_;
  idf::GPIO_Output reset_;

  friend class BootLoaderSession<RawBLDriver>;

  void SetModeBootLoader() { this->boot0_.set_high(); }
  void UnsetModeBootLoader() { this->boot0_.set_low(); }

 public:
  Session(std::shared_ptr<RawBLDriver> raw_bl_driver, idf::GPIONum boot0,
          idf::GPIONum reset)
      : raw_bl_driver_(raw_bl_driver), boot0_(boot0), reset_(reset) {
    this->boot0_.set_low();
    this->reset_.set_high();
  }

  void Reset() {
    this->reset_.set_low();
    vTaskDelay(50 / portTICK_PERIOD_MS);

    this->reset_.set_high();
  }

  BootLoaderSession<RawDriver> EnterBL() {
    return BootLoaderSession(this->bl_driver_, this->shared_from_this());
  }
};
}  // namespace stm32
