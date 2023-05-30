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

  void DataUpdate(int cid, uint8_t* value, size_t len) {
    uint8_t buf[10];
    buf[0] = 0x02;
    buf[1] = (cid >> 24) & 0xff;
    buf[2] = (cid >> 16) & 0xff;
    buf[3] = (cid >> 8) & 0xff;
    buf[4] = (cid >> 0) & 0xff;

    buf[5] = (len >> 24) & 0xff;
    buf[6] = (len >> 16) & 0xff;
    buf[7] = (len >> 8) & 0xff;
    buf[8] = (len >> 0) & 0xff;

    uart.Send(buf, 9);

    uart.Send(value, len);

    return;
  }

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

  TaskResult Idle() {
    auto buffer_length = uart.GetRXBufferDataLength();
    if (buffer_length > 0) {
      ESP_LOGI(TAG, "RX buffer length: %d", buffer_length);
    } else {
      return TaskResult::Ok();
    }

    RUN_TASK(uart.RecvChar(1000 / portTICK_PERIOD_MS), op);

    switch (op) {
      case 2: {
        uint8_t cid_buf[4];
        uint8_t len_buf[4];

        RUN_TASK_V(this->ReadExactly(cid_buf, 4, 200 / portTICK_PERIOD_MS));
        RUN_TASK_V(this->ReadExactly(len_buf, 4, 200 / portTICK_PERIOD_MS));

        uint32_t cid =
            cid_buf[0] << 24 | cid_buf[1] << 16 | cid_buf[2] << 8 | cid_buf[3];
        uint32_t len =
            len_buf[0] << 24 | len_buf[1] << 16 | len_buf[2] << 8 | len_buf[3];

        ESP_LOGI(TAG, "Data update: cid=%ld, len=%ld", cid, len);

        uint8_t* data = (uint8_t*)malloc(len);
        RUN_TASK_V(this->ReadExactly(data, len, 200 / portTICK_PERIOD_MS));

        // TODO: Do notify Date update

        free(data);
        break;
      }
    }
    return TaskResult::Ok();
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
    dbg.Idle();
    if (gpio_get_level(GPIO_NUM_18) == 0) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    while (gpio_get_level(GPIO_NUM_18) == 1) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

    dbg.DataUpdate(1, data, 4);
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