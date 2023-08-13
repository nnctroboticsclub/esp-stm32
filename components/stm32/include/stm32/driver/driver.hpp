#pragma once

#include <vector>
#include <memory>

#include "types/commands.hpp"
#include "types/version.hpp"
#include "types/flash_page.hpp"
#include "types/error.hpp"

#include "../raw_driver/raw_driver.hpp"

namespace stm32::driver {
using raw_driver::InboundData;
using raw_driver::OutboundData;

template <typename P>
  requires raw_driver::RawDriverConcept<P>
class Driver {
  static constexpr const char *TAG = "stm32bl Driver";

  std::shared_ptr<P> raw_driver_;

  Version version;
  Commands commands;

  void Get() {
    this->raw_driver_->CommandHeader(this->commands.get);

    std::vector<uint8_t> buf(2);
    this->raw_driver_->Recv(
        (raw_driver::InboundData){.data = buf, .resume = false});

    std::vector<uint8_t> raw_commands(buf[0]);
    this->raw_driver_->Recv(
        (raw_driver::InboundData){.data = raw_commands, .resume = true});
    this->raw_driver_->ACK();

    this->version.UpdateVersion(buf[1]);
    this->commands = Commands(raw_commands);

    return;
  }

  void GetVersion() {
    this->raw_driver_->CommandHeader(this->commands.get_version);

    std::vector<uint8_t> buf(3);
    this->raw_driver_->Recv(
        (raw_driver::InboundData){.data = buf, .resume = true});
    this->version = Version(buf);

    this->raw_driver_->ACK();

    return;
  }

 public:
  void InitConnection() {
    this->Get();
    this->GetVersion();

    return;
  }

  void Erase(SpecialFlashPage page) {
    ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
    if (this->commands.UseLegacyErase()) {
      this->raw_driver_->CommandHeader(this->commands.erase);

      this->raw_driver_->Send(OutboundData::U16WithChecksum((uint16_t)page));
    } else {  //! Legacy Erase
      // TODO(syoch): Impl
      throw NotImplemented();
    }
  }

  void Erase(const std::vector<FlashPage> &pages) {
    ESP_LOGI(TAG, "Erasing %d pages", pages.size());
    ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
             0x0800'0000 + pages[pages.size() - 1] * 0x800);
    if (this->commands.UseLegacyErase()) {
      this->raw_driver_->CommandHeader(this->commands.erase);

      this->raw_driver_->SendData(OutboundData::U16WithChecksum(pages.size()));

      std::vector<uint8_t> buf(pages.size() * 2, 0x77);
      for (size_t i = 0; i < pages.size(); i++) {
        buf[2 * i] = pages[i] >> 8;
        buf[2 * i + 1] = pages[i] & 0xff;
      }
      OutboundData packet_2{
          .data = buf,
          .size = OutboundData::SizeMode::kNone,
          .checksum = OutboundData::ChecksumMode::kData,
          // cspell: disable-next-line
          .checksum_base = 0x5A  // WHAT IS 0x5A (NANNMO-WAKARAN)
      };
      this->raw_driver_->SendData(packet_2);

    } else {  //! Legacy Erase
      // TODO(syoch): Impl
      throw NotImplemented();
    }
  }

  void WriteMemoryBlock(uint32_t address, const std::vector<uint8_t> &buffer) {
    this->CommandHeader(this->commands.write_memory);

    this->SendData(OutboundData::U32WithChecksum(address));

    OutboundData packet2{
        .data = buffer,
        .size = OutboundData::SizeMode::kU8,
        .checksum = OutboundData::ChecksumMode::kWithLength,
    };
    this->SendData(packet2);

    return;
  }

  void Go(uint32_t address) {
    ESP_LOGI(TAG, "Go to %08lx", address);
    this->CommandHeader(this->commands.go);

    this->SendData(OutboundData::U32WithChecksum(address));

    return;
  }
};
}  // namespace stm32::driver
