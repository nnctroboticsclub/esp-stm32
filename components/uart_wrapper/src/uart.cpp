#include "uart.hpp"
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <memory.h>
#include <sdkconfig.h>

UART::UART(uart_port_t port, int tx, int rx, int baud_rate,
           uart_parity_t parity)
    : port(port), buf_size(1024 * 2), tx(tx), rx(rx) {
  uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = parity,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
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
#ifdef CONFIG_UART_WRAPPER_ENABLE_DEBUG
  printf("*%d*, %d )--> \n  ", tx, rx);
  if (size > 30) {
    printf("(*snip*)");
  } else {
    for (size_t i = 0; i < size; i++) {
      printf("%02x ", buf[i]);
      if (i % 16 == 15) {
        printf("\n  ");
      }
    }
  }
  printf("\n");
#endif
  return uart_write_bytes(this->port, buf, size);
}
Result<ssize_t> UART::Recv(uint8_t* buf, size_t size, TickType_t timeout) {
  memset((void*)buf, 0, size);

  size_t bytes = uart_read_bytes(this->port, (void*)buf, size, timeout);
  if (bytes == 0) {
    ESP_LOGE(TAG, "Failed to receive data (timeout)");
    return ESP_ERR_TIMEOUT;
  }

#ifdef CONFIG_UART_WRAPPER_ENABLE_DEBUG
  printf("%d, *%d* )<-- \n  ", tx, rx);
  if (size > 30) {
    printf("(*snip*)");
  } else {
    for (size_t i = 0; i < size; i++) {
      printf("%02x ", buf[i]);
      if (i % 16 == 15) {
        printf("\n  ");
      }
    }
  }
  printf("\n");
#endif
  return Result<ssize_t>::Ok(bytes);
}

Result<uint8_t> UART::RecvChar(TickType_t timeout) {
  uint8_t c = 0;
  RUN_TASK_V(this->Recv((uint8_t*)&c, 1, timeout));
  return Result<uint8_t>::Ok(c);
}
void UART::SendChar(uint8_t ch) {
  uint8_t c = ch;
  this->Send(&c, 1);
}

TaskResult UART::RecvExactly(uint8_t* buf, size_t size, TickType_t timeout) {
  size_t read = 0;
  while (read < size) {
    RUN_TASK(this->Recv(buf + read, size - read, timeout), ret);

    read += ret;
  }

  return TaskResult::Ok();
}