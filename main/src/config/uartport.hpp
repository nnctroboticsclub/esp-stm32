#pragma once

#include <string>
#include <memory>
#include <vector>
#include <connection/data_link/uart.hpp>

#include "../libs/nvs_proxy.hpp"

namespace nvs {
template <>
class AliasProxyTable<uart_port_t> {
  using type = uint8_t;
};

template <>
class AliasProxyTable<uart_parity_t> {
  using type = uint8_t;
};
}  // namespace nvs

namespace profile {
class UartPort : public nvs::Namespace {
  std::shared_ptr<connection::data_link::UART> dev;

  nvs::Proxy<uart_port_t> uart_port;
  nvs::Proxy<gpio_num_t> tx;
  nvs::Proxy<gpio_num_t> rx;
  nvs::Proxy<uint32_t> baud_rate;
  nvs::Proxy<uart_parity_t> parity;

 public:
  explicit UartPort(std::string const& ns);

  std::shared_ptr<connection::data_link::UART> GetDevice() const;
};
}  // namespace profile
