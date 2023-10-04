#pragma once

#include <memory>
#include <utility>
#include <string>

#include <stream/datalink/uart.hpp>

class UartStreaming {
  using Device = stream::datalink::UART;

  uint16_t id = -1;
  std::shared_ptr<Device> uart;

  static void HandleUart(void *);

 public:
  UartStreaming(std::string name);

  UartStreaming(std::string name, uart_port_t port, int tx, int rx,
                int baud_rate, uart_parity_t parity)
      : UartStreaming(name) {
    this->uart = std::make_shared<Device>(port, tx, rx, baud_rate, parity);
    this->uart->OnReceive(UartStreaming::HandleUart, this);
  }
};