#pragma once

#include "../../libs/nvs_proxy.hpp"

namespace types {
enum NetworkMode : uint8_t { None = 0, STA, AP };
}

namespace nvs {
template <>
class Proxy<types::NetworkMode> : public Proxy<uint8_t> {
 public:
  using Proxy<uint8_t>::Proxy;

  operator types::NetworkMode() {
    types::NetworkMode value;
    value = (types::NetworkMode)Proxy<uint8_t>::operator uint8_t();
    return value;
  }

  Proxy& operator=(types::NetworkMode value) {
    Proxy<uint8_t>::operator=((uint8_t)value);
    return *this;
  }
};
}  // namespace nvs