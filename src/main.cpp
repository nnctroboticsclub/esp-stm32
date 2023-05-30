#include <esp_log.h>

#include "./config.hpp"
#include "init/init.hpp"
#include "bin.h"

using namespace app;

#define USER_PROGRAM_START 0x08000000

const char* TAG = "Main";

class DebuggerMaster {
  static constexpr const char* TAG = "DebuggerMaster";
  UART uart;

  std::optional<std::vector<uint8_t>> ui_cache;

 private:
  TaskResult ReadExactly(uint8_t* buf, size_t size,
                         TickType_t timeout = portMAX_DELAY) {
    size_t read = 0;
    while (read < size) {
      RUN_TASK(uart.Recv(buf + read, size - read, timeout), ret);

      read += ret;
    }

    return TaskResult::Ok();
  }

 public:
  DebuggerMaster(uart_port_t port, int tx, int rx)
      : uart(port, tx, rx, 9600, UART_PARITY_DISABLE), ui_cache(std::nullopt) {}

  Result<std::vector<uint8_t>> GetUI() {
    ESP_LOGI(TAG, "Getting UI");
    if (ui_cache.has_value()) {
      ESP_LOGI(TAG, "Using cached UI");
      return Result<std::vector<uint8_t>>::Ok(ui_cache.value());
    }

    uart.Send((uint8_t*)"\x01", 1);

    RUN_TASK(this->uart.RecvChar(1000 / portTICK_PERIOD_MS), c);
    if (c != 0) {
      ESP_LOGE(TAG, "Invalid UI command: %#02x", c);
      return Result<std::vector<uint8_t>>::Err(ESP_ERR_INVALID_RESPONSE);
    }

    uint8_t length_buf[4]{};

    RUN_TASK_V(this->ReadExactly(length_buf, 4, 200 / portTICK_PERIOD_MS));

    uint32_t length = length_buf[0] << 24 | length_buf[1] << 16 |
                      length_buf[2] << 8 | length_buf[3];
    ESP_LOGI(TAG, "UI length: %ld", length);

    ui_cache = std::vector<uint8_t>(length);

    RUN_TASK_V(this->ReadExactly(ui_cache.value().data(), length));

    ESP_LOGI(TAG, "UI received");
    return Result<std::vector<uint8_t>>::Ok(ui_cache.value());
  }
};
DebuggerMaster dbg(UART_NUM_2, 17, 16);

void BootStrap() {
  // init::init_serial();
  // init::init_mdns();
  // init::init_data_server();
}

void Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();

  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
  while (1) {
    if (gpio_get_level(GPIO_NUM_18) == 0) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    while (gpio_get_level(GPIO_NUM_18) == 1) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    auto res = dbg.DateUpdate(1, 2);
    if (res.IsErr()) {
      ESP_LOGE(TAG, "Failed to get UI: %s", esp_err_to_name(res.Error()));
      continue;
    }

    for (auto& byte : *res) {
      printf("%02x ", byte);
    }
    printf("\n");
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