#include <stm32/driver/driver.hpp>

#include <stm32/raw_driver/raw_driver.hpp>

#include <vector>
#include <memory>

namespace stm32::driver {

using raw_driver::InboundData;
using raw_driver::OutboundData;

void BLDriver::Get() {
  this->raw_driver_->CommandHeader(this->commands.get);

  std::vector<uint8_t> buf = this->raw_driver_->Recv(2);

  std::vector<uint8_t> raw_commands = this->raw_driver_->Recv(buf[0], true);
  this->raw_driver_->ACK();

  this->version.UpdateVersion(buf[1]);
  this->commands = Commands(raw_commands);
}

void BLDriver::DoGetVersion() {
  this->raw_driver_->CommandHeader(this->commands.get_version);

  std::vector<uint8_t> buf = this->raw_driver_->Recv(3);
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
  this->commands = Commands();
  this->version = Version();

  this->raw_driver_->Sync();
  this->is_connected_ = true;
  this->Get();
  this->GetVersion();

  return;
}

Version BLDriver::GetVersion() {
  if (!this->is_connected_) {
    this->InitConnection();
  }

  return this->version;
}

void BLDriver::Erase(SpecialFlashPage page) const {
  if (this->commands.UseLegacyErase()) {
    this->raw_driver_->CommandHeader(this->commands.erase);

    this->raw_driver_->Send(OutboundData::U16WithChecksum((uint16_t)page));
  } else {  //! Legacy Erase
    // TODO(syoch): Impl
    throw NotImplemented();
  }
}

void BLDriver::Erase(std::vector<FlashPage> const &pages) const {
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

std::shared_ptr<BLDriver> BLDriver::SPIDriver(idf::SPIMaster &master,
                                              idf::CS chip_select) {
  return std::make_shared<BLDriver>(
      std::make_shared<raw_driver::SPIRawDriver>(master, chip_select));
}

}  // namespace stm32::driver
