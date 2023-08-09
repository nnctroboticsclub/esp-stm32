#pragma once

#include <cinttypes>
#include <vector>

namespace stm32::raw_driver {
class Checksum {
  uint8_t checksum = 0;

 public:
  Checksum(uint8_t initialize_value = 0x00) : checksum(initialize_value){};

  inline Checksum &operator<<(std::vector<uint8_t> const &data) {
    for (auto x : data) {
      *this << x;
    }
    return *this;
  }

  inline Checksum &operator<<(uint8_t x) {
    this->checksum ^= x;
    return *this;
  }

  inline Checksum &operator<<(uint16_t x) {
    *this << uint8_t(x >> 8);
    *this << uint8_t(x & 0xff);
    return *this;
  }
  inline Checksum &operator<<(uint32_t x) {
    *this << uint16_t(x >> 16);
    *this << uint16_t(x & 0xffff);
    return *this;
  }

  inline Checksum &operator<<(Checksum const &x) {
    *this << uint8_t(x);
    return *this;
  }

  inline void Reset() { this->checksum = 0; }

  explicit operator uint8_t() const { return this->checksum; }
};

}  // namespace stm32::raw_driver
