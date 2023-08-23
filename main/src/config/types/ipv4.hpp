#pragma once

#include <cinttypes>

#include <esp_netif_ip_addr.h>

namespace types {
using Ipv4 = uint32_t;

inline esp_ip4_addr ToEspIp4Addr(Ipv4 ip) {
  esp_ip4_addr addr;
  addr.addr = ip;
  return addr;
}

}  // namespace types