#include "init.hpp"

#include <esp_log.h>
#include <mdns.h>

void init::init_mdns() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

#ifdef CONFIG_USE_MDNS
  init_wifi();

  ESP_LOGI(TAG, "Initializing mDNS");

  ESP_ERROR_CHECK(mdns_init());

  ESP_LOGI(TAG, "- 1/3 Set hostname...");
  mdns_hostname_set(CONFIG_MDNS_HOSTNAME);
  mdns_instance_name_set("ESP32");
  ESP_LOGI(TAG, "- 2/3 Add service...");
  mdns_service_add(NULL, "_tcp", "_tcp", 4007, 2, 0);
  ESP_LOGI(TAG, "- 3/3 mDNS is setted up!...");
#else
  ESP_LOGW(TAG, "mDNS is disabled.");
#endif
}