#include "init.hpp"

#include "../config.hpp"

#include <esp_log.h>

void init::init_serial() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  ESP_LOGI(TAG, "Initializing Serial pins");

  // TODO: Fix this or remove this
  // config::tx.Init();
  // config::rx.Init();
}