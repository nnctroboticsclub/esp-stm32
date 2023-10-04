#pragma once

#include <string>
#include <memory>
#include <data_proxy/handler.hpp>

#include "../libs/nvs_proxy.hpp"
#include "types/bus_type.hpp"

namespace profile {
class DataProxyProfile : public nvs::Namespace {
  nvs::Proxy<uint8_t> id;
  nvs::Proxy<types::BusType> bus_type;
  nvs::Proxy<uint8_t> bus_port;
  nvs::Proxy<uint8_t> cs;
  std::shared_ptr<esp_stm32::data_proxy::Handler> cache;

 public:
  explicit DataProxyProfile(std::string const &ns)
      : nvs::Namespace(ns),
        id(this, "id"),
        bus_type(this, "bus_type"),
        bus_port(this, "bus_port"),
        cs(this, "cs") {}
  std::shared_ptr<esp_stm32::data_proxy::Handler> Get();

  inline uint8_t GetID() { return id; }

  inline DataProxyProfile &SetID(uint8_t id_) {
    this->id = id_;
    return *this;
  }

  inline DataProxyProfile &SetBusType(types::BusType type) {
    bus_type = type;
    return *this;
  }

  inline DataProxyProfile &SetBusPort(uint8_t port) {
    bus_port = port;
    return *this;
  }

  inline DataProxyProfile &SetCS(uint8_t cs_) {
    cs = cs_;
    return *this;
  }
};
}  // namespace profile
