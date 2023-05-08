#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <sdkconfig.h>
#include "libs/wifi.hpp"
#include "./config.hpp"
#include <esp_log.h>
#include "server.hpp"
#include "init/init.hpp"

#include <optional>

#define WRITING_BAUD 115200

using namespace app;

const char* TAG = "Main";

void BootStrap() {
  init::init_mdns();
  init::init_serial();

  ESP_LOGI(TAG, "Starting Network EventLoop Task.");
  config::network.StartEventLoop();
}

void Main() {
  Server server;
  ESP_LOGI(TAG, "Starting Server");
  ESP_LOGI(TAG, "  - bind...");
  if (!server.bind(4007)) {
    ESP_LOGE(TAG, "Failed to bind server.");
    return;
  }
  ESP_LOGI(TAG, "  - listen...");
  if (!server.listen()) {
    ESP_LOGE(TAG, "Failed to listen server.");
    return;
  }
  ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  server.ClientLoop();
}

extern "C" void app_main() {
  BootStrap();
  Main();
}