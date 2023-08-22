#pragma once

#include <string>
#include <memory>

#include "../libs/nvs_proxy.hpp"
#include "types/network_mode.hpp"
#include "types/ip_mode.hpp"
#include "types/ipv4.hpp"

namespace profile {
class NetworkProfile : public nvs::Namespace {  // a_nw%d
 public:
  [[no_unique_address]] nvs::Proxy<types::NetworkMode> mode;
  [[no_unique_address]] nvs::Proxy<types::IPMode> ip_mode;

  [[no_unique_address]] nvs::Proxy<std::string> name;

  [[no_unique_address]] nvs::Proxy<std::string> ssid;
  [[no_unique_address]] nvs::Proxy<std::string> password;

  [[no_unique_address]] nvs::Proxy<std::string> hostname;

  [[no_unique_address]] nvs::Proxy<types::Ipv4> ip;
  [[no_unique_address]] nvs::Proxy<types::Ipv4> subnet;
  [[no_unique_address]] nvs::Proxy<types::Ipv4> gateway;

 public:
  explicit NetworkProfile(std::string const& ns);
};
}  // namespace profile