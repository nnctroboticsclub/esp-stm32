#pragma once

#include "../libs/nvs_proxy.hpp"
#include "types/ipv4.hpp"

namespace profile {
class ServerProfile {
 public:
  nvs::Proxy<types::Ipv4> ip;
  nvs::Proxy<uint16_t> port;

  ServerProfile(nvs::Namespace* ns);
};
}  // namespace profile