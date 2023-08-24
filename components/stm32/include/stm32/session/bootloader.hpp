#pragma once

#include <memory>

#include <gpio_cxx.hpp>
#include "./stm32.hpp"
#include "../driver/driver.hpp"
#include "../raw_driver/types/error.hpp"
#include "../raw_driver/impl/base.hpp"

namespace stm32::session {

class BootLoaderSession {
  static constexpr const char *TAG = "[STM32-BL] STM32";
  std::shared_ptr<driver::BLDriver> bl_driver_;
  std::shared_ptr<STM32> session_;

  int failed_attempts_ = 0;

  void Reset() const;

 public:
  BootLoaderSession(std::shared_ptr<driver::BLDriver> bl_driver,
                    std::shared_ptr<STM32> session);

  void WriteMemory(uint32_t address, std::vector<uint8_t> &buf);
  void Erase(driver::ErasePages pages);
  void Go(uint32_t address);
  driver::Version GetVersion();
};
}  // namespace stm32::session
