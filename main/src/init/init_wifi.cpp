#include "init.hpp"

#include <esp_log.h>

#include "../libs/wifi.hpp"
#include "../config.hpp"

void init::init_wifi() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

#ifdef USE_NETWORK
  init_netif();
  init_eventloop();

  ESP_LOGI(TAG, "Initializing Wi-Fi");

  config::network.Init();
  ESP_LOGI(TAG, "- 1/4 Setup...");
  config::network.Setup();
  ESP_LOGI(TAG, "- 2/4 Connect...");
  config::network.Connect();
  ESP_LOGI(TAG, "- 3/4 Waiting connection...");
  config::network.WaitConnection();
  ESP_LOGI(TAG, "- 4/4 Wifi is setted up!...");

  ESP_LOGI(TAG, "Starting Network EventLoop Task.");
  config::network.StartEventLoop();
#else
  ESP_LOGW(TAG, "Wi-Fi is disabled.");
#endif
}