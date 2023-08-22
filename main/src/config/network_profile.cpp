#include "./network_profile.hpp"

namespace profile {
using nvs::Namespace;
NetworkProfile::NetworkProfile(std::string const& ns)
    : Namespace(ns),
      mode(this, "mode"),
      ip_mode(this, "ip_mode"),
      name(this, "name"),
      ssid(this, "ssid"),
      password(this, "password"),
      hostname(this, "hostname"),
      ip(this, "ip"),
      subnet(this, "subnet"),
      gateway(this, "gateway") {}
}  // namespace profile