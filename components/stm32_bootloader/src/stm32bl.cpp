#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stm32bl/presentation.hpp>

#include <helper.hpp>

#include <cmath>

using namespace connection::presentation::stm32bl;

STM32BootLoader::STM32BootLoader(idf::GPIONum reset, idf::GPIONum boot0)
    : session(reset, boot0) {}

STM32BootLoader::~STM32BootLoader() = default;

// Commands

void STM32BootLoader::WriteMemoryBlock(uint32_t address,
                                       std::vector<uint8_t> &buffer) {
  this->CommandHeader(this->commands.write_memory);

  OutboundData packet1{
      .data = ToU8Vector(address),
      .size = OutboundData::SizeMode::kNone,
      .checksum = OutboundData::ChecksumMode::kData,
  };
  this->SendData(packet1);

  OutboundData packet2{
      .data = buffer,
      .size = OutboundData::SizeMode::kU8,
      .checksum = OutboundData::ChecksumMode::kWithLength,
  };
  this->SendData(packet2);

  return;
}

void STM32BootLoader::Go(uint32_t address) {
  ESP_LOGI(TAG, "Go to %08lx", address);
  this->CommandHeader(this->commands.go);

  OutboundData packet1{
      .data = ToU8Vector(address),
      .size = OutboundData::SizeMode::kNone,
      .checksum = OutboundData::ChecksumMode::kData,
  };
  this->SendData(packet1);

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