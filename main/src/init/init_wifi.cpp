#include "init.hpp"

#include "../config.hpp"

#include <esp_log.h>
#include <nvs_flash.h>

void init::init_wifi() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  auto config = config::Config::GetInstance();
  auto profile = config->network_profiles[config->active_network_profile];

  char* name = profile.name;
  ESP_LOGI(TAG, "Initializing WiFi [%s]", name);
  config::network.InitHW();
  config::network.SetCredentials(profile.ssid, profile.password);
  if (profile.ip_mode == types::IPMode::STATIC) {
    config::network.SetIP(profile.ip.ToEspIp4Addr(),
                          profile.subnet.ToEspIp4Addr(),
                          profile.gateway.ToEspIp4Addr());
  }

  if (profile.mode == types::NetworkMode::AP) {
    config::network.InitAp();
  } else {
    config::network.InitSta();
  }
}