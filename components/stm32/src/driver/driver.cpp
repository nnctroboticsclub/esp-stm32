#include <stm32/driver/driver.hpp>

#include <vector>
#include <memory>

namespace stm32::driver {

using raw_driver::InboundData;
using raw_driver::OutboundData;

void BLDriver::Get() {
  ESP_LOGI(TAG, "Doing Get...");
  this->raw_driver_->CommandHeader(this->commands.get);

  std::vector<uint8_t> buf(2);
  ESP_LOGI(TAG, "Getting Header...");
  this->raw_driver_->Recv((InboundData){.data = buf, .resume = false});

  std::vector<uint8_t> raw_commands(buf[0]);
  ESP_LOGI(TAG, "Getting Version/Commands...");
  this->raw_driver_->Recv((InboundData){.data = raw_commands, .resume = true});
  this->raw_driver_->ACK();

  this->version.UpdateVersion(buf[1]);
  this->commands = Commands(raw_commands);

  return;
}

void BLDriver::DoGetVersion() {
  ESP_LOGI(TAG, "Doing Get Version...");
  this->raw_driver_->CommandHeader(this->commands.get_version);

  std::vector<uint8_t> buf(3);
  this->raw_driver_->Recv((InboundData){.data = buf, .resume = true});
  this->version = Version(buf);

  this->raw_driver_->ACK();

  return;
}

//* public

BLDriver::BLDriver(std::shared_ptr<RawDriverBase> raw_driver)
    : raw_driver_(raw_driver) {
  return;
}

void BLDriver::InitConnection() {
  this->raw_driver_->Sync();
  this->Get();
  this->GetVersion();
  this->is_connected_ = true;

  return;
}

Version BLDriver::GetVersion() {
  if (!this->is_connected_) {
    this->InitConnection();
  }

  return this->version;
}

void BLDriver::Erase(SpecialFlashPage page) const {
  ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
  if (this->commands.UseLegacyErase()) {
    this->raw_driver_->CommandHeader(this->commands.erase);

    this->raw_driver_->Send(OutboundData::U16WithChecksum((uint16_t)page));
  } else {  //! Legacy Erase
    // TODO(syoch): Impl
    throw NotImplemented();
  }
}

void BLDriver::Erase(std::vector<FlashPage> const &pages) const {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
           0x0800'0000 + pages[pages.size() - 1] * 0x800);
  if (this->commands.UseLegacyErase()) {
    this->raw_driver_->CommandHeader(this->commands.erase);

    this->raw_driver_->Send(OutboundData::U16WithChecksum(pages.size()));

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
    this->raw_driver_->Send(packet_2);

  } else {  //! Legacy Erase
    // TODO(syoch): Impl
    throw NotImplemented();
  }
}

void BLDriver::WriteMemoryBlock(uint32_t address,
                                const std::vector<uint8_t> &buffer) const {
  this->raw_driver_->CommandHeader(this->commands.write_memory);

  this->raw_driver_->Send(OutboundData::U32WithChecksum(address));

  OutboundData packet2{
      .data = buffer,
      .size = OutboundData::SizeMode::kU8,
      .checksum = OutboundData::ChecksumMode::kWithLength,
  };
  this->raw_driver_->Send(packet2);

  return;
}

void BLDriver::Go(uint32_t address) const {
  ESP_LOGI(TAG, "Go to %08lx", address);
  this->raw_driver_->CommandHeader(this->commands.go);

  this->raw_driver_->Send(OutboundData::U32WithChecksum(address));

  return;
}

}  // namespace stm32::driver