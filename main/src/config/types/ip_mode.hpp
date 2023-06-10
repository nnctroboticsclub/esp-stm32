#pragma once

#include "../../libs/nvs_proxy.hpp"

namespace types {
enum IPMode : uint8_t {
  DHCP = 0,
  STATIC,
};
}

namespace nvs {
template <>
class Proxy<types::IPMode> : public Proxy<uint8_t> {
 public:
  using Proxy<uint8_t>::Proxy;

  operator types::IPMode() {
    types::IPMode value;
    value = (types::IPMode)Proxy<uint8_t>::operator uint8_t();
    return value;
  }

  Proxy& operator=(types::IPMode value) {
    Proxy<uint8_t>::operator=((uint8_t)value);
    return *this;
  }
};
}  // namespace nvs