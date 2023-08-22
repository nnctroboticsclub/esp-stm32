#pragma once

#include "../../libs/nvs_proxy.hpp"

namespace types {
enum class NetworkMode : uint8_t { None = 0, STA, AP };
}

namespace nvs {
template <>
class AliasProxyTable<types::NetworkMode> {
  using type = uint8_t;
};
}  // namespace nvs