#include <connection/data_link/uart.hpp>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <memory>
#include <sdkconfig.h>

namespace connection::data_link {
UART::UART(uart_port_t port) : port(port) {}

void UART::InstallDriver(int tx, int rx, int baud_rate,
                         uart_parity_t parity) const {
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

  ESP_ERROR_CHECK(uart_driver_install(this->port, 2 * 1024, 0, 10, nullptr, 0));
}

size_t UART::GetRXBufferDataLength() const {
  size_t size;
  uart_get_buffered_data_len(this->port, &size);
  return size;
}

void UART::Flush() const { uart_flush_input(this->port); }

size_t UART::Send(const std::vector<uint8_t> &buf) noexcept {
  if (this->IsTraceEnabled())
    ESP_LOG_BUFFER_HEX("UART-->", buf.data(), buf.size());

  return uart_write_bytes(this->port, buf.data(), buf.size());
}

size_t UART::Recv(std::vector<uint8_t> &buf, TickType_t timeout) {
  std::ranges::fill(buf.begin(), buf.end(), 0);

  size_t bytes =
      uart_read_bytes(this->port, (void *)buf.data(), buf.size(), timeout);
  if (bytes == 0) {
    ESP_LOGE(TAG, "Failed to receive data (timeout)");
    return ESP_ERR_TIMEOUT;
  }

  if (this->IsTraceEnabled())
    ESP_LOG_BUFFER_HEX("UART<--", buf.data(), buf.size());

  return bytes;
}
}  // namespace connection::data_link