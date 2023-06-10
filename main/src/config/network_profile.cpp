#include "network_profile.hpp"
namespace profile {
NetworkProfile::NetworkProfile(nvs::Namespace* ns)
    : ns_(ns),
      mode(ns, "mode"),
      ip_mode(ns, "ip_mode"),
      name(ns, "name"),
      ssid(ns, "ssid"),
      password(ns, "password"),
      hostname(ns, "hostname"),
      ip(ns, "ip"),
      subnet(ns, "subnet"),
      gateway(ns, "gateway") {}

void NetworkProfile::Save() { this->ns_->Commit(); }
}  // namespace profile