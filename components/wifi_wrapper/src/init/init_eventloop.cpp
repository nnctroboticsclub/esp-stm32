#include "init.hpp"

#include <esp_log.h>
#include <esp_event.h>

void wifi_init::init_eventloop() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  ESP_LOGI(TAG, "Initializing Event Loop");

  ESP_ERROR_CHECK(esp_event_loop_create_default());
}