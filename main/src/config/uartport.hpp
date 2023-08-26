#pragma once

#include <string>
#include <memory>
#include <vector>
#include <connection/data_link/uart.hpp>

#include "../libs/nvs_proxy.hpp"

namespace nvs {
template <>
struct AliasProxyTable<uart_parity_t> {
  using type = uint8_t;
};
}  // namespace nvs

namespace profile {
class UartPort : public nvs::Namespace {
  std::shared_ptr<connection::data_link::UART> dev;

  nvs::Proxy<uint8_t> port;
  nvs::Proxy<gpio_num_t> tx;
  nvs::Proxy<gpio_num_t> rx;
  nvs::Proxy<uint32_t> baud_rate;
  nvs::Proxy<uart_parity_t> parity;

 public:
  explicit UartPort(std::string const& ns);

  std::shared_ptr<connection::data_link::UART> GetDevice() const;

  inline uint8_t GetPort() const { return port; }

  inline UartPort& SetPort(uint8_t port_) {
    this->port = port_;
    return *this;
  }
  inline UartPort& SetTx(gpio_num_t pin) {
    tx = pin;
    return *this;
  }
  inline UartPort& SetRx(gpio_num_t pin) {
    rx = pin;
    return *this;
  }
  inline UartPort& SetBaudRate(uint32_t baud_rate_) {
    this->baud_rate = baud_rate_;
    return *this;
  }
  inline UartPort& SetParity(uart_parity_t parity_) {
    this->parity = parity_;
    return *this;
  }
};
}  // namespace profile
