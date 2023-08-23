#pragma once

#include "../../libs/nvs_proxy.hpp"

namespace types {
enum class BusType : uint8_t { None = 0, SPI, UART, Debug };
}

namespace nvs {

template <>
struct AliasProxyTable<types::BusType> {
  using type = uint8_t;
};
}  // namespace nvs