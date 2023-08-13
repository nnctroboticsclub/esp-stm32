#pragma once

#include <memory>

#include <gpio_cxx.hpp>
#include "../driver/driver.hpp"
#include "../raw_driver/types/error.hpp"
#include "../raw_driver/impl/base.hpp"

namespace stm32 {

template <typename P>
  requires raw_driver::RawDriverConcept<P>
class Session;

template <raw_driver::RawDriverConcept RawDriver>
  requires STM32Session<Session<RawDriver>>
class BootLoaderSession {
  using Driver = driver::Driver<RawDriver>;
  using ParentSession = Session<RawDriver>;

  std::shared_ptr<Driver> bl_driver_;
  std::shared_ptr<ParentSession> session_;

  int failed_attempts_ = 0;

  void Sync() {
    int attempts = 0;
    this->session_->SetModeBootLoader();
    while (true) {
      try {
        this->session_->Reset();
        this->bl_driver_->InitConnection();
        break;
      } catch (const raw_driver::ConnectionDisrupted &) {
        if (attempts++ > 10) {
          throw raw_driver::ConnectionDisrupted();
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
      }
    }
    this->session_->UnsetModeBootLoader();

    this->bl_driver_->InitConnection();

    return;
  }

 public:
  BootLoaderSession(std::shared_ptr<RawDriver> bl_driver,
                    std::shared_ptr<ParentSession> session)
      : bl_driver_(bl_driver), session_(session) {}

  void WriteMemory(uint32_t address, std::array<uint8_t, 256> &buf) {
    failed_attempts_++;
    if (failed_attempts_ > 10) {
      throw raw_driver::ConnectionDisrupted();
    }
    try {
      this->bl_driver_->WriteMemory(address, buf);

      failed_attempts_ = 0;
    } catch (const raw_driver::ACKFailed &e) {
      this->Sync();
      this->WriteMemory(address, buf);
    }
  }
  void Erase(driver::ErasePages pages) {
    failed_attempts_++;
    if (failed_attempts_ > 10) {
      throw raw_driver::ConnectionDisrupted();
    }
    try {
      this->bl_driver_->Erase(pages);

      failed_attempts_ = 0;
    } catch (const raw_driver::ACKFailed &e) {
      this->Sync();
      this->Erase(pages);
    }
  }
  driver::Version GetVersion() {
    failed_attempts_++;
    if (failed_attempts_ > 10) {
      throw raw_driver::ConnectionDisrupted();
    }
    try {
      driver::Version version = this->bl_driver_->GetVersion();

      failed_attempts_ = 0;
      return version;
    } catch (const raw_driver::ACKFailed &e) {
      this->Sync();
      return this->GetVersion();
    }
  }
};
}  // namespace stm32
