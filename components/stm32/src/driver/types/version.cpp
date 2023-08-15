#pragma once

#include <stm32/driver/types/version.hpp>

#include <cinttypes>
#include <vector>

namespace stm32::driver {

Version::Version() = default;

Version::Version(const std::vector<uint8_t> &data) {
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

}  // namespace stm32::driver
