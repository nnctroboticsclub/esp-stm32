#pragma once

#include <memory>

#include <gpio_cxx.hpp>
#include "driver/driver.hpp"
#include "raw_driver/types/error.hpp"

namespace stm32 {

template <typename P>
  requires raw_driver::RawDriverConcept<P>
class Session;

template <typename RawDriver>
  requires raw_driver::RawDriverConcept<RawDriver>
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
      }
    }
    this->session_->UnsetModeBootLoader();

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

template <typename RawDriver>
  requires raw_driver::RawDriverConcept<RawDriver>
class Session {
  std::shared_ptr<RawDriver> raw_bl_driver_;
  idf::GPIO_Output boot0_;
  idf::GPIO_Output reset_;

  bool is_in_bl_mode_ = false;

  friend class BootLoaderSession<RawDriver>;

  void SetModeBootLoader() { this->boot0_.set_high(); }
  void UnsetModeBootLoader() { this->boot0_.set_low(); }

 public:
  Session(std::shared_ptr<RawDriver> raw_bl_driver, idf::GPIONum boot0,
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
