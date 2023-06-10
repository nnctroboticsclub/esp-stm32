#include "init.hpp"

#include <esp_log.h>
#include <esp_event.h>

#include "../config.hpp"

void init::init_data_server() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  init_wifi();

  ESP_LOGI(TAG, "Initializing DataServer(TCP[4007])");
  config::server.MakeSocket();
  ESP_LOGI(TAG, "  - bind...");
  if (!config::server.bind(4007)) {
    ESP_LOGE(TAG, "Failed to bind server.");
    return;
  }
  ESP_LOGI(TAG, "  - listen...");
  if (!config::server.listen()) {
    ESP_LOGE(TAG, "Failed to listen server.");
    return;
  }
}