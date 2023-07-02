#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace connection::application::stm32bl {
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
  vTaskDelay(50 / portTICK_PERIOD_MS);
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

      ESP_LOGI(TAG, "Erasing 4 sub-pages...");
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