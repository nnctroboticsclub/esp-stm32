#pragma once

#include <inttypes.h>
#include "../../libs/nvs_proxy.hpp"

#include <esp_netif_ip_addr.h>

namespace types {
union Ipv4 {
  uint32_t ip;
  uint8_t ip_bytes[4];
};
}  // namespace types

namespace nvs {
template <>
class Proxy<types::Ipv4> : public Proxy<uint32_t> {
 public:
  using Proxy<uint32_t>::Proxy;

  operator types::Ipv4() {
    types::Ipv4 value;
    value.ip = Proxy<uint32_t>::operator uint32_t();
    return value;
  }

  Proxy& operator=(types::Ipv4 value) {
    Proxy<uint32_t>::operator=(value.ip);
    return *this;
  }

  Proxy& operator=(uint32_t value) {
    *this = (types::Ipv4){.ip = value};
    return *this;
  }

  esp_ip4_addr_t ToEspIp4Addr() {
    esp_ip4_addr_t addr;
    addr.addr = Proxy<uint32_t>::operator uint32_t();
    return addr;
  }
};

}  // namespace nvs