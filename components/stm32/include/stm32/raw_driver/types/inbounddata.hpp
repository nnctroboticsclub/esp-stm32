#pragma once

#include <cinttypes>
#include <vector>

namespace stm32::raw_driver {
struct InboundData {
  //* Datas
  size_t length;

  bool resume = false;
};

}  // namespace stm32::raw_driver
