#pragma once

#include <string>

#include "../libs/nvs_proxy.hpp"
#include "types/ipv4.hpp"

namespace profile {
class ServerProfile : public nvs::Namespace {
 public:
  nvs::Proxy<types::Ipv4> ip;
  nvs::Proxy<uint16_t> port;

  explicit ServerProfile(std::string const& ns);
};
}  // namespace profile