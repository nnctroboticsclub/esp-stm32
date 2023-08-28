#include <stm32/session/bootloader.hpp>

#include <stm32/session/stm32.hpp>

namespace stm32::session {

void BootLoaderSession::Reset() const {
  int attempts = 0;
  this->session_->SetModeBootLoader();
  while (true) {
    try {
      ESP_LOGI(TAG, "Try %d", attempts);
      this->session_->Reset();
      vTaskDelay(200 / portTICK_PERIOD_MS);

      this->bl_driver_->InitConnection();
      break;
    } catch (const raw_driver::SyncFailed &) {
      if (attempts++ > 2) {
        this->session_->UnsetModeBootLoader();
        throw raw_driver::SyncFailed();
      }

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
  this->session_->UnsetModeBootLoader();

  return;
}
BootLoaderSession::BootLoaderSession(
    std::shared_ptr<driver::BLDriver> bl_driver, std::shared_ptr<STM32> session)
    : bl_driver_(bl_driver), session_(session), failed_attempts_(0) {
  this->Reset();  // This function must be called at least once
}

void BootLoaderSession::WriteMemory(uint32_t address,
                                    std::vector<uint8_t> &buf) {
  while (true) {
    try {
      auto it = buf.begin();
      auto addr = address;
      auto addr_end = address + buf.size();

      while (it != buf.end()) {
        auto it_next = std::min(buf.end(), it + 256);

        if ((addr - address) % 0x1000 == 0) {
          auto end = std::min(addr_end, addr + 0x1000);
          ESP_LOGI(TAG, "Writing Data to %08lx --> %08lx [%3ld%%]", addr, end,
                   (100 * (end - address)) / (addr_end - address));
        }

        std::vector<uint8_t> sub_buf(it, it_next);
        this->bl_driver_->WriteMemoryBlock(addr, sub_buf);

        addr = std::min(addr_end, addr + 256);
        it = it_next;
      }

      this->failed_attempts_ = 0;
      break;
    } catch (const raw_driver::ACKFailed &e) {
      this->failed_attempts_++;
      if (this->failed_attempts_ > 10) {
        throw raw_driver::ConnectionDisrupted();
      }
      this->Reset();

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}

void BootLoaderSession::Erase(driver::ErasePages pages) {
  while (true) {
    try {
      for (const auto &page : pages.special_pages) {
        this->bl_driver_->Erase(page);
      }

      if (!pages.normal_pages.empty()) {
        // this->bl_driver_->Erase(pages.normal_pages);
        int all_pages = pages.normal_pages.size();

        auto it = pages.normal_pages.begin();
        int i = 0;
        for (i = 0; i + 4 < pages.normal_pages.size(); i += 4, it += 4) {
          std::vector<uint16_t> sub_pages(it, it + 4);

          ESP_LOGI(TAG, "Erasing 4 pages... [%d %d %d %d]", *it, *(it + 1),
                   *(it + 2), *(it + 3));
          this->bl_driver_->Erase(sub_pages);
        }

        if (all_pages % 4 != 0) {
          std::vector<uint16_t> sub_pages(pages.normal_pages.begin() + i,
                                          pages.normal_pages.end());
          std::string pages_str = "";
          for (auto &&page : sub_pages) {
            pages_str += std::to_string(page) + " ";
          }
          pages_str.resize(pages_str.length() - 1);

          ESP_LOGI(TAG, "Erasing %d pages... [%s]", sub_pages.size(),
                   pages_str.c_str());
          this->bl_driver_->Erase(sub_pages);
        }
      }

      this->failed_attempts_ = 0;
      break;
    } catch (const raw_driver::ACKFailed &) {
      this->failed_attempts_++;
      if (this->failed_attempts_ > 10) {
        throw raw_driver::ConnectionDisrupted();
      }
      this->Reset();

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}
void BootLoaderSession::Go(uint32_t address) {
  while (true) {
    try {
      this->bl_driver_->Go(address);
      this->failed_attempts_ = 0;
      break;
    } catch (const raw_driver::ACKFailed &) {
      this->failed_attempts_++;
      if (this->failed_attempts_ > 10) {
        throw raw_driver::ConnectionDisrupted();
      }
      this->Reset();

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}
driver::Version BootLoaderSession::GetVersion() {
  while (true) {
    try {
      auto version = this->bl_driver_->GetVersion();
      this->failed_attempts_ = 0;
      return version;
    } catch (const raw_driver::ACKFailed &e) {
      this->failed_attempts_++;
      if (this->failed_attempts_ > 10) {
        throw raw_driver::ConnectionDisrupted();
      }
      this->Reset();

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}
}  // namespace stm32::session
