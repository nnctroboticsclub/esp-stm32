#include "uart.hpp"
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <memory.h>

UART::UART(uart_port_t port, int tx, int rx, int baud_rate,
           uart_parity_t parity)
    : port(port), buf_size(1024 * 2) {
  uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = parity,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_param_config(this->port, &uart_config));
  uart_set_pin(this->port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  ESP_ERROR_CHECK(
      uart_driver_install(this->port, this->buf_size, 0, 10, nullptr, 0));
}

size_t UART::GetRXBufferDataLength() {
  size_t size;
  uart_get_buffered_data_len(this->port, &size);
  return size;
}

void UART::Flush() { uart_flush_input(this->port); }
size_t UART::Send(uint8_t* buf, size_t size) {
  // printf("--> ");
  // if (size > 30) {
  //   printf("(*snip*)");
  // } else {
  //   for (size_t i = 0; i < size; i++) {
  //     printf("%02x ", buf[i]);
  //   }
  // }
  // printf("\n");
  return uart_write_bytes(this->port, buf, size);
}
size_t UART::Recv(uint8_t* buf, size_t size, TickType_t timeout) {
  memset((void*)buf, 0, size);
  size_t bytes = uart_read_bytes(this->port, (void*)buf, size, timeout);
  // printf("<- ");
  // if (size > 30) {
  //   printf("(*snip*)");
  // } else {
  //   for (size_t i = 0; i < size; i++) {
  //     printf("%02x ", buf[i]);
  //   }
  // }
  // printf("\n");
  if (bytes == 0) {
    ESP_LOGE(TAG, "Failed to receive data (timeout)");
    return -1;
  }
  return bytes;
}

uint8_t UART::RecvChar(TickType_t timeout) {
  uint8_t c = 0;
  auto ret = this->Recv((uint8_t*)&c, 1, timeout);
  if (ret < 0) {
    return -1;
  } else if (ret == 0) {
    return -1;
  }
  return c;
}
void UART::SendChar(uint8_t ch) {
  uint8_t c = ch;
  this->Send(&c, 1);
}