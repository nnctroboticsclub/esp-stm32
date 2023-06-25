#pragma once

#include <driver/uart.h>
#include <string>

#include "result.hpp"

class UART {
  static constexpr const char* TAG = "UART";
  uart_port_t port;
  size_t buf_size;
  int tx, rx;

 private:
  static void EventLoop(void*);

 public:
  UART(uart_port_t port, int tx, int rx, int baud_rate, uart_parity_t parity);
  UART(uart_port_t port);

  size_t GetRXBufferDataLength();

  void Flush();

  size_t Send(uint8_t* buf, size_t size);
  Result<ssize_t> Recv(uint8_t* buf, size_t size, TickType_t timeout);

  Result<uint8_t> RecvChar(TickType_t timeout = 1000 / portTICK_PERIOD_MS);
  void SendChar(uint8_t);

  TaskResult RecvExactly(uint8_t* buf, size_t size,
                         TickType_t timeout = portMAX_DELAY);

  template <size_t N>
  size_t Send(const char (&buf)[N]) {
    return Send((uint8_t*)buf, N - 1);
  }

  template <size_t N>
  TaskResult RecvExactly(uint8_t (&buf)[N],
                         TickType_t timeout = portMAX_DELAY) {
    return RecvExactly(buf, N, timeout);
  }

  Result<std::string> RecvUntil(char delim,
                                TickType_t timeout = portMAX_DELAY) {
    std::string ret;
    while (true) {
      RUN_TASK(this->RecvChar(timeout), c);
      if (c == delim) {
        return ret;
      }
      ret += c;
    }
    return ret;
  }
};