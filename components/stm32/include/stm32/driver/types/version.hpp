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

  Version();
  explicit Version(const std::vector<uint8_t> &data);

  void UpdateVersion(uint8_t byte);
};

}  // namespace stm32::driver
