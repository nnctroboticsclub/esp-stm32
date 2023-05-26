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
  void ReadExactly(uint8_t* buf, size_t size,
                   TickType_t timeout = portMAX_DELAY) {
    size_t read = 0;
    while (read < size) {
      read += uart.Recv(buf + read, size - read, timeout);
    }
  }

 public:
  DebuggerMaster(uart_port_t port, int tx, int rx)
      : uart(port, tx, rx, 9600, UART_PARITY_DISABLE), ui_cache(std::nullopt) {}

  std::vector<uint8_t>& GetUI() {
    ESP_LOGI(TAG, "Getting UI");
    if (ui_cache.has_value()) {
      return ui_cache.value();
    }

    uart.Send((uint8_t*)"\x01", 1);

    uint8_t length_buf[4]{};
    this->ReadExactly(length_buf, 4);
    uint32_t length = length_buf[0] << 24 | length_buf[1] << 16 |
                      length_buf[2] << 8 | length_buf[3];
    ESP_LOGI(TAG, "UI length: %ld", length);

    ui_cache = std::vector<uint8_t>(length);
    this->ReadExactly(ui_cache.value().data(), length);

    ESP_LOGI(TAG, "UI received");
    return ui_cache.value();
  }
};
DebuggerMaster dbg(UART_NUM_2, 5, 4);

void BootStrap() {
  // init::init_serial();
  // init::init_mdns();
  // init::init_data_server();
}

void Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();
  auto ui = dbg.GetUI();
  for (auto& byte : ui) {
    printf("%#02x ", byte);
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