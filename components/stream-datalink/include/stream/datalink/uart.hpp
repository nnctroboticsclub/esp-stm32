#pragma once

#include <driver/uart.h>

#include <string>
#include <stdexcept>
#include <vector>

#include <stream-base.hpp>

namespace stream::datalink {

class UARTError : public std::runtime_error {
 public:
  UARTError(const char *msg) : std::runtime_error(msg) {}
};

class UART : public RecvAndSend {
  static constexpr const char *TAG = "UART";
  uart_port_t port;

 private:
  static void EventLoop(void *);

 public:
  UART(uart_port_t port, int tx, int rx, int baud_rate, uart_parity_t parity);

  size_t GetRXBufferDataLength() const;

  void Flush() const;

  size_t Send(const std::vector<uint8_t> &buf) noexcept override;

  size_t Recv(std::vector<uint8_t> &buf, TickType_t timeout) override;
};
}  // namespace stream