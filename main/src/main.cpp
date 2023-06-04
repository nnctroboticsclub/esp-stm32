#include <esp_log.h>

#include "./config.hpp"
#include "init/init.hpp"
#include "bin.h"

using namespace app;

const char* TAG = "Main";

void BootStrap() {
  // init::init_serial();
  init::init_mdns();
  init::init_data_server();
}

void Main() {
  ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  config::server.StartClientLoopAtForeground();
}

extern "C" int app_main() {
  BootStrap();
  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}