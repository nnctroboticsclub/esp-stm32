#include <esp_log.h>

#include "./config.hpp"
#include "init/init.hpp"
#include "bin.h"

using namespace app;

#define USER_PROGRAM_START 0x08000000

const char* TAG = "Main";

void UARTReadLoop(void* param) {
  uint8_t* data = (uint8_t*)malloc(1024);
  while (1) {
    int len = uart_read_bytes(UART_NUM_2, data, 1024, 100 / portTICK_PERIOD_MS);
    if (len > 0) {
      ESP_LOGI(TAG, "Read %d bytes", len);
      for (int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
        if (i % 16 == 15) {
          printf("\n");
        }
      }
      printf("\n");
    }
  }
  free(data);
}

void BootStrap() {
  // init::init_serial();
  // init::init_mdns();
  // init::init_data_server();

  uart_config_t uart_config = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_ODD,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

  QueueHandle_t uart_queue;
  uart_set_pin(UART_NUM_2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  ESP_ERROR_CHECK(
      uart_driver_install(UART_NUM_2, 1024 * 2, 0, 10, &uart_queue, 0));
  xTaskCreate(UARTReadLoop, "UARTReadLoop", 1024 * 2, NULL, 10, NULL);
}

void Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();

  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
  while (1) {
    if (gpio_get_level(GPIO_NUM_18) != 1) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    ESP_LOGI(TAG, "Send a command...");
    char bin[] = "\xff\xfb\xfa\xfd\x01";
    uart_write_bytes(UART_NUM_2, (const char*)bin, sizeof(bin));
    while (gpio_get_level(GPIO_NUM_18) == 1) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Waiting for the response...");
  }
}

extern "C" void app_main() {
  BootStrap();
  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}