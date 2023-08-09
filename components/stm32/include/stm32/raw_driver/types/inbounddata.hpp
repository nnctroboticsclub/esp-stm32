#pragma once

#include <cinttypes>
#include <vector>

namespace stm32::raw_driver {
struct InboundData {
  //* Datas
  std::vector<uint8_t> data = {};

  bool resume = false;
};
}  // namespace stm32::raw_driver
