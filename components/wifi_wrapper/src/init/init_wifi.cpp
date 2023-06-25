#include "init.hpp"

#include <esp_log.h>
#include <esp_wifi.h>

#include <wifi.hpp>

void wifi::init::init_wifi_lib(esp_event_handler_t event_handler, void* arg) {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  init_eventloop();

  ESP_LOGI(TAG, "Initializing Wi-Fi Lib");
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  cfg.nvs_enable = false;
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_LOGI(TAG, "Registering WIFI_EVENT");
  esp_event_handler_instance_t wifi_handler;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, arg, &wifi_handler));

  ESP_LOGI(TAG, "Registering IP_EVENT");
  esp_event_handler_instance_t ip_handler;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, arg, &ip_handler));
}