#include <esp_log.h>

#include "./config.hpp"
#include "init/init.hpp"
#include "bin.h"

using namespace app;

#define USER_PROGRAM_START 0x08000000

const char* TAG = "Main";

void BootStrap() {
  // init::init_serial();
  init::init_mdns();
  init::init_data_server();

  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_EVEN,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

  QueueHandle_t uart_queue;
  uart_set_pin(UART_NUM_2, 32, 33, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  ESP_ERROR_CHECK(
      uart_driver_install(UART_NUM_2, 1024 * 2, 0, 10, &uart_queue, 0));
}

void Main() {
  ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  config::server.StartClientLoopAtForeground();
}

extern "C" void app_main() {
  BootStrap();
  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}