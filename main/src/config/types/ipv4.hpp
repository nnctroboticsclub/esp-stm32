#pragma once

#include <cinttypes>

#include <esp_netif_ip_addr.h>

namespace types {
using Ipv4 = uint32_t;

inline esp_ip4_addr ToEspIp4Addr(Ipv4 ip) {
  esp_ip4_addr addr;
  addr.addr = ((ip & 0xFF000000) >> 24) | ((ip & 0x00FF0000) >> 8) |
              ((ip & 0x0000FF00) << 8) | ((ip & 0x000000FF) << 24);
  return addr;
}

}  // namespace types