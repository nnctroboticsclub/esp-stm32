#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cmath>

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
    : session(reset, boot0) {}

STM32BootLoader::~STM32BootLoader() = default;

void STM32BootLoader::Connect() {
  ESP_LOGI(TAG, "Connect...");
  this->session.TurnOnBoot1();
  while (true) {
    this->session.Reset();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    try {
      this->Sync();
    } catch (ACKFailed &) {
      ESP_LOGE(TAG, "Failed to connect to STM32 Bootloader");
      continue;
    }
    break;
  }
  this->session.TurnOffBoot1();

  this->Get();
}

// Commands

void STM32BootLoader::WriteMemoryBlock(uint32_t address,
                                       std::vector<uint8_t> &buffer) {
  this->CommandHeader(this->commands.write_memory);
  this->SendAddress(address);
  this->SendDataWithChecksum(buffer);

  return;
}

void STM32BootLoader::Go(uint32_t address) {
  ESP_LOGI(TAG, "Go to %08lx", address);
  this->CommandHeader(this->commands.go);

  this->SendAddress(address);

  return;
}

void STM32BootLoader::Get() {
  this->CommandHeader(this->commands.get);

  std::vector<uint8_t> buf(2);
  this->ReadData(buf);

  std::vector<uint8_t> raw_command(buf[0]);
  this->ReadDataWithoutHeader(raw_command);
  this->RecvACK();

  this->version.UpdateVersion(buf[1]);
  this->commands = Commands(raw_command);

  return;
}

void STM32BootLoader::GetVersion() {
  this->CommandHeader(this->commands.get_version);

  std::vector<uint8_t> buf(3);
  this->ReadDataWithoutHeader(buf);
  this->version = Version(buf);

  this->RecvACK();

  return;
}

void STM32BootLoader::Erase(SpecialFlashPage page) {
  ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
  if (this->commands.UseLegacyErase()) {
    this->CommandHeader(this->commands.erase);
    this->SendFlashPage(page);
  } else {  //! Legacy Erase
    // TODO(syoch): Impl
    throw NotImplemented();
  }
}

void STM32BootLoader::Erase(std::vector<FlashPage> &pages) {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
           0x0800'0000 + pages[pages.size() - 1] * 0x800);
  if (this->commands.UseLegacyErase()) {
    this->CommandHeader(this->commands.erase);
    this->SendFlashPage(pages);
  } else {  //! Legacy Erase
    // TODO(syoch): Impl
    throw NotImplemented();
  }
}

// Utility functions

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

void STM32BootLoader::WriteMemory(uint32_t address,
                                  std::vector<uint8_t> &data) {
  int remains = data.size();
  auto it = data.begin();
  auto ptr = address;

  while (remains > 0) {
    ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", ptr,
             std::min(remains, 256 * 8));

    for (size_t i = 0; i < 8; i++) {
      std::vector chunk(it, it + std::min(remains, 256));
      this->WriteMemoryBlock(ptr, chunk);
      remains = std::max(remains - chunk.size(), 0u);

      it += 256;
      ptr += 256;

      if (remains <= 0) break;
    }
  }
  return;
}

}  // namespace connection::application::stm32bl