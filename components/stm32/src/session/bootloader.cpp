#include <stm32/session/bootloader.hpp>

#include <stm32/session/stm32.hpp>

namespace stm32::session {

void BootLoaderSession::Sync() {
  int attempts = 0;
  ESP_LOGI(TAG, "Syncing...");
  this->session_->SetModeBootLoader();
  while (true) {
    try {
      this->session_->Reset();
      ESP_LOGI(TAG, "Initializing Connection...");
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

  return;
}
BootLoaderSession::BootLoaderSession(
    std::shared_ptr<raw_driver::RawDriverBase> bl_driver,
    std::shared_ptr<Session> session)
    : session_(session) {
  this->bl_driver_ = std::make_shared<driver::BLDriver>(bl_driver);
  this->Sync();  // This function must be called at least once
}

void BootLoaderSession::WriteMemory(uint32_t address,
                                    std::vector<uint8_t> &buf) {
  while (true) {
    try {
      auto it = buf.begin();
      auto addr = address;

      while (it != buf.end()) {
        std::vector<uint8_t> sub_buf(it, it + 256);
        this->bl_driver_->WriteMemoryBlock(addr, sub_buf);
        addr += 256;
        it += 256;
      }

      this->failed_attempts_ = 0;
      break;
    } catch (const raw_driver::ACKFailed &e) {
      this->failed_attempts_++;
      if (this->failed_attempts_ > 10) {
        throw raw_driver::ConnectionDisrupted();
      }
      this->Sync();

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

          ESP_LOGI(TAG, "Erasing 4 sub-pages.normal_pages... [%d %d %d %d]",
                   *it, *(it + 1), *(it + 2), *(it + 3));
          this->bl_driver_->Erase(sub_pages);
        }

        if (all_pages % 4 != 0) {
          std::vector<uint16_t> sub_pages(pages.normal_pages.begin() + i,
                                          pages.normal_pages.end());
          ESP_LOGI(TAG, "Erasing pages... (All %d pages)", all_pages);
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
      this->Sync();

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
      this->Sync();

      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}
}  // namespace stm32::session
