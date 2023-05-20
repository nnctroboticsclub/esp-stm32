#include <driver/uart.h>

class UART {
  static constexpr const char* TAG = "UART";
  uart_port_t port;
  size_t buf_size;

 private:
  static void EventLoop(void*);

 public:
  UART(uart_port_t port, int tx, int rx, int baud_rate);

  size_t GetRXBufferDataLength();

  void Flush();

  size_t Send(const char* buf, size_t size);
  size_t Recv(const char* buf, size_t size, TickType_t timeout);

  uint8_t RecvChar(TickType_t timeout = 1000 / portTICK_PERIOD_MS);
  void SendChar(uint8_t);
};