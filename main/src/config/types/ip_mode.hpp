#pragma once

#include "../../libs/nvs_proxy.hpp"

namespace types {
enum class IPMode : uint8_t {
  DHCP = 0,
  STATIC,
};
}

namespace nvs {
template <>
struct AliasProxyTable<types::IPMode>{ using type = uint8_t; };
}  // namespace nvs