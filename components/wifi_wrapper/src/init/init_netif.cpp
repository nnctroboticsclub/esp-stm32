#include "init.hpp"

#include <esp_log.h>
#include <esp_netif.h>

void wifi_init::init_netif() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  init_nvs();

  ESP_LOGI(TAG, "Initializing Net If");
  ESP_ERROR_CHECK(esp_netif_init());
}