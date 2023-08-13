#pragma once

#include <cinttypes>
#include <vector>

namespace stm32::raw_driver {
struct OutboundData {
  enum class SizeMode { kNone = 0, kU16, kU8 };
  enum class ChecksumMode {
    kUnused = 0,
    kData,
    kWithLength,
  };
  //* Datas
  std::vector<uint8_t> data;

  SizeMode size = SizeMode::kNone;
  ChecksumMode checksum = ChecksumMode::kUnused;

  uint8_t checksum_base = 0x00;

  uint8_t CalculateChecksum() const;

  static OutboundData U32WithChecksum(uint32_t x) {
    OutboundData data;
    data.data =
        std::vector<uint8_t>{(uint8_t)(x >> 0x18), (uint8_t)(x >> 0x10),
                             (uint8_t)(x >> 0x08), (uint8_t)(x >> 0x00)};
    data.size = SizeMode::kNone;
    data.checksum = ChecksumMode::kData;
    return data;
  }

  static OutboundData U16WithChecksum(uint16_t x) {
    OutboundData data;
    data.data =
        std::vector<uint8_t>{(uint8_t)(x >> 0x08), (uint8_t)(x >> 0x00)};
    data.size = SizeMode::kNone;
    data.checksum = ChecksumMode::kData;
    return data;
  }
};
}  // namespace stm32::raw_driver
