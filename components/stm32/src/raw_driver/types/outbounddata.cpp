#include <stm32/raw_driver/types/outbounddata.hpp>

#include <cassert>
#include <stm32/raw_driver/types/checksum.hpp>

namespace stm32::raw_driver {

uint8_t OutboundData::CalculateChecksum() const {
  using enum OutboundData::SizeMode;
  using enum OutboundData::ChecksumMode;

  Checksum data_length_checksum;
  switch (this->size) {
    case kNone:
      break;
    case kU8:
      assert(this->data.size() <= 0x100);
      data_length_checksum << (uint8_t)(this->data.size() - 1);
      break;
    case kU16:
      assert(this->data.size() <= 0xffff);
      data_length_checksum << (uint16_t)this->data.size();
      break;
  }

  Checksum checksum;
  checksum << this->checksum_base;

  switch (this->checksum) {
    case kUnused:
      break;
    case kWithLength:
      checksum << data_length_checksum;
      [[fallthrough]];

    case kData:
      checksum << this->data;

      break;
  }

  return (uint8_t)checksum;
}

}  // namespace stm32::raw_driver
