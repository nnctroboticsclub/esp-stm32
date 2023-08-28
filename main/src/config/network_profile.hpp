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
  [[no_unique_address]] nvs::Proxy<uint8_t> id;

  [[no_unique_address]] nvs::Proxy<types::NetworkMode> mode;
  [[no_unique_address]] nvs::Proxy<types::IPMode> ip_mode;

  [[no_unique_address]] nvs::Proxy<std::string> ssid;
  [[no_unique_address]] nvs::Proxy<std::string> password;

  [[no_unique_address]] nvs::Proxy<std::string> hostname;

  [[no_unique_address]] nvs::Proxy<types::Ipv4> ip;
  [[no_unique_address]] nvs::Proxy<types::Ipv4> subnet;
  [[no_unique_address]] nvs::Proxy<types::Ipv4> gateway;

  explicit NetworkProfile(std::string const& ns);

  inline uint8_t GetID() { return this->id.Get(); }
  inline NetworkProfile& SetID(uint8_t id_) {
    this->id.Set(id_);
    return *this;
  }

  inline types::NetworkMode GetMode() { return this->mode.Get(); }
  inline NetworkProfile& SetMode(types::NetworkMode mode_) {
    this->mode.Set(mode_);
    return *this;
  }

  inline types::IPMode GetIPMode() { return this->ip_mode.Get(); }
  inline NetworkProfile& SetIPMode(types::IPMode ip_mode_) {
    this->ip_mode.Set(ip_mode_);
    return *this;
  }

  inline std::string GetSSID() { return this->ssid.Get(); }
  inline NetworkProfile& SetSSID(std::string const& ssid_) {
    this->ssid.Set(ssid_);
    return *this;
  }

  inline std::string GetPassword() { return this->password.Get(); }
  inline NetworkProfile& SetPassword(std::string const& password_) {
    this->password.Set(password_);
    return *this;
  }

  inline std::string GetHostname() { return this->hostname.Get(); }
  inline NetworkProfile& SetHostname(std::string const& hostname_) {
    this->hostname.Set(hostname_);
    return *this;
  }

  inline types::Ipv4 GetIP() { return this->ip.Get(); }
  inline NetworkProfile& SetIP(types::Ipv4 ip_) {
    this->ip.Set(ip_);
    return *this;
  }

  inline types::Ipv4 GetSubnet() { return this->subnet.Get(); }
  inline NetworkProfile& SetSubnet(types::Ipv4 subnet_) {
    this->subnet.Set(subnet_);
    return *this;
  }

  inline types::Ipv4 GetGateway() { return this->gateway.Get(); }
  inline NetworkProfile& SetGateway(types::Ipv4 gateway_) {
    this->gateway.Set(gateway_);
    return *this;
  }
};
}  // namespace profile