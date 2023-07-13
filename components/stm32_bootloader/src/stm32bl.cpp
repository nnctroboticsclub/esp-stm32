#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace connection::application::stm32bl {

Commands::Commands(std::vector<uint8_t> &data) {
  if (data.size() < 0x0b) {
    ESP_LOGE("Stm32BL", "data size is too small: %d bytes", data.size());

    throw InvalidSize();
  }
  this->get = data[0];
  this->get_version = data[1];
  this->get_id = data[2];
  this->read_memory = data[3];
  this->go = data[4];
  this->write_memory = data[5];
  this->erase = data[6];
  this->write_protect = data[7];
  this->write_unprotect = data[8];
  this->readout_protect = data[9];
  this->readout_unprotect = data[10];
  if (data.size() > 0x0c) {
    this->get_checksum = data[11];
  }
}

Version::Version(std::vector<uint8_t> &data) {
  if (data.empty()) {
    this->major = 0;
    this->minor = 0;
    this->option1 = 0;
    this->option2 = 0;
    return;
  }
  this->UpdateVersion(data[0]);

  if (data.size() > 1) {
    this->option1 = data[1];
  }

  if (data.size() > 2) {
    this->option1 = data[2];
  }
}

void Version::UpdateVersion(uint8_t byte) {
  this->major = byte >> 4;
  this->minor = byte & 0x0f;
}

STM32BootLoader::STM32BootLoader(idf::GPIONum reset, idf::GPIONum boot0)
    : reset(reset), boot0(boot0) {
  this->reset.set_high();
  this->boot0.set_low();
}

STM32BootLoader::~STM32BootLoader() = default;

void STM32BootLoader::BootBootLoader() {
  ESP_LOGI(TAG, "Booting Bootloader");

  this->boot0.set_high();
  vTaskDelay(100 / portTICK_PERIOD_MS);

  this->reset.set_low();
  vTaskDelay(100 / portTICK_PERIOD_MS);

  this->reset.set_high();
  vTaskDelay(100 / portTICK_PERIOD_MS);

  this->boot0.set_low();
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void STM32BootLoader::Erase(uint32_t address, uint32_t length) {
  auto pages = MemoryRangeToPages(address, length);
  this->Erase(pages);
}

void STM32BootLoader::Erase(Pages pages) {
  if (pages.bank1 && pages.bank2) {
    this->Erase(SpecialFlashPage::kGlobal);
  } else if (pages.bank1) {
    this->Erase(SpecialFlashPage::kBank1);
  } else if (pages.bank2) {
    this->Erase(SpecialFlashPage::kBank2);
  }

  if (!pages.pages.empty()) {
    int all_pages = pages.pages.size();

    auto it = pages.pages.begin();
    int i = 0;
    for (i = 0; i + 4 < pages.pages.size(); i += 4, it += 4) {
      std::vector<uint16_t> sub_pages(it, it + 4);

      ESP_LOGI(TAG, "Erasing 4 sub-pages... [%d %d %d %d]", *it, *(it + 1),
               *(it + 2), *(it + 3));
      this->Erase(sub_pages);
    }

    if (all_pages % 4 != 0) {
      std::vector<uint16_t> sub_pages(pages.pages.begin() + i,
                                      pages.pages.end());
      ESP_LOGI(TAG, "Erasing pages... (All %d pages)", all_pages);
      this->Erase(sub_pages);
    }
  }
}

void STM32BootLoader::WriteMemory(uint32_t address, std::vector<uint8_t> &buf) {
  int remains = buf.size();
  auto it = buf.begin();
  auto ptr = address;

  while (remains > 0) {
    std::vector sub_buffer(it, it + (remains > 256 ? 256 : remains));
    this->WriteMemoryBlock(ptr, sub_buffer);
    remains = remains - 256 > 0 ? remains - 256 : 0;
    it += 256;
    ptr += 256;
  }
  return;
}

}  // namespace connection::application::stm32bl