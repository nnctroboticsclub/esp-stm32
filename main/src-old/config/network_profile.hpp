#pragma once

#include <memory>

#include "../libs/nvs_proxy.hpp"
#include "types/network_mode.hpp"
#include "types/ip_mode.hpp"
#include "types/ipv4.hpp"

namespace profile {
class NetworkProfile {  // a_nw%d
 private:
  nvs::SharedNamespace ns_;

 public:
  nvs::Proxy<types::NetworkMode> mode;
  nvs::Proxy<types::IPMode> ip_mode;

  nvs::Proxy<char[20]> name;

  nvs::Proxy<char[20]> ssid;
  nvs::Proxy<char[20]> password;

  nvs::Proxy<char[20]> hostname;

  nvs::Proxy<types::Ipv4> ip;
  nvs::Proxy<types::Ipv4> subnet;
  nvs::Proxy<types::Ipv4> gateway;

 public:
  NetworkProfile(nvs::SharedNamespace ns);
  void Save();
};
}  // namespace profile