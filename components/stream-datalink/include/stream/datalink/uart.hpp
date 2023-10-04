#pragma once

#include <driver/uart.h>

#include <string>
#include <stdexcept>
#include <vector>
#include <functional>
#include <utility>

#include <stream-base.hpp>

namespace stream::datalink {

class UARTError : public std::runtime_error {
 public:
  UARTError(const char *msg) : std::runtime_error(msg) {}
};

class UART : public RecvAndSend {
  static constexpr const char *TAG = "UART";
  uart_port_t port;
  QueueHandle_t uart_queue;

  using Callback = std::function<void(void *)>;

  struct CallbackObject {
    Callback function;
    void *arg;

    void operator()() {
      if (this->function) this->function(this->arg);
    }
  };

  std::vector<CallbackObject> rx_callbacks;

 private:
  TaskHandle_t uart_task;
  static void UARTTask(void *);

 public:
  UART(uart_port_t port, int tx, int rx, int baud_rate, uart_parity_t parity);

  size_t GetRXBufferDataLength() const;

  void Flush() const;

  size_t Send(const std::vector<uint8_t> &buf) noexcept override;

  size_t Recv(std::vector<uint8_t> &buf, TickType_t timeout) override;

  void OnReceive(std::function<void(void *)> function, void *arg);
};
}  // namespace stream::datalink