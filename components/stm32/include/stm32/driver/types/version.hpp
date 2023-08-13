#pragma once

#include <cinttypes>
#include <vector>

namespace stm32::driver {
struct Version {
  bool updated = false;
  uint8_t major = 0xee;
  uint8_t minor = 0xee;
  uint8_t option1 = 0xee;
  uint8_t option2 = 0xee;

  Version() = default;
  explicit Version(const std::vector<uint8_t> &data) {
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

  void UpdateVersion(uint8_t byte) {
    this->major = byte >> 4;
    this->minor = byte & 0x0f;
  }
};

}  // namespace stm32::driver
