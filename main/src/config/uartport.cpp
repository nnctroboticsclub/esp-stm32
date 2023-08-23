#include "uartport.hpp"

namespace profile {

UartPort::UartPort(std::string const& ns)
    : nvs::Namespace(ns),
      uart_port(this, "port"),
      tx(this, "tx"),
      rx(this, "rx"),
      baud_rate(this, "rate"),
      parity(this, "prt") {
  if ((gpio_num_t)this->tx != 0) {
    this->dev = std::make_shared<connection::data_link::UART>(
        (uart_port_t)this->uart_port.Get(), (gpio_num_t)this->tx,
        (gpio_num_t)this->rx, (uint32_t)this->baud_rate,
        (uart_parity_t)this->parity);
  }
}

std::shared_ptr<connection::data_link::UART> UartPort::GetDevice() const {
  if (this->dev == nullptr) {
    throw std::runtime_error("UART Port not initialised");
  }
  return dev;
}

}  // namespace profile