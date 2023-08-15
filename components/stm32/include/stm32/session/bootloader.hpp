#pragma once

#include <memory>

#include <gpio_cxx.hpp>
#include "./stm32.hpp"
#include "../driver/driver.hpp"
#include "../raw_driver/types/error.hpp"
#include "../raw_driver/impl/base.hpp"

namespace stm32::session {

class BootLoaderSession {
  static constexpr const char *TAG = "[STM32-BL] Session";
  std::shared_ptr<driver::BLDriver> bl_driver_;
  std::shared_ptr<Session> session_;

  int failed_attempts_ = 0;

  void Sync();

 public:
  BootLoaderSession(std::shared_ptr<raw_driver::RawDriverBase> bl_driver,
                    std::shared_ptr<Session> session);

  void WriteMemory(uint32_t address, std::vector<uint8_t> &buf);
  void Erase(driver::ErasePages pages);
  driver::Version GetVersion();
};
}  // namespace stm32::session
