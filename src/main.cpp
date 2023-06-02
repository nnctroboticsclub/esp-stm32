#include <esp_log.h>

#include "./config.hpp"
#include "init/init.hpp"
#include "bin.h"
// #include <lwip/sockets.h>
using namespace app;

#define USER_PROGRAM_START 0x08000000

const char* TAG = "Main";

//

void BootStrap() {
  // init::init_serial();
  // init::init_mdns();
  init::init_data_server();
}

void Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();

  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
  while (1) {
    // dbg.Idle();
    if (gpio_get_level(GPIO_NUM_18) == 0) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    while (gpio_get_level(GPIO_NUM_18) == 1) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

    // dbg.DataUpdate(1, data, 4);
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