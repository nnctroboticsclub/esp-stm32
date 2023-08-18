#include "init.hpp"

#include "../config/config.hpp"

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
  ESP_LOGI(TAG, "SSID: %s, password: %s", (char*)profile.ssid,
           (char*)profile.password);
  if (profile.ip_mode == types::IPMode::STATIC) {
    esp_netif_ip_info_t ip_info{
        .ip = profile.ip.ToEspIp4Addr(),
        .netmask = profile.subnet.ToEspIp4Addr(),
        .gw = profile.gateway.ToEspIp4Addr(),
    };
    config::network.SetIP(ip_info);
  }

  if (profile.mode == types::NetworkMode::AP) {
    config::network.InitAp(profile.ssid, profile.password);

    config::network.WaitUntilConnected();
  } else {
    wifi::WifiConnectionProfile prof{
        .auth_mode = WIFI_AUTH_WPA2_PSK,
        .ssid = (char*)profile.ssid,
        .password = (char*)profile.password,
        .user = "",
        .id = "",
    };
    config::network.InitSta();
    config::network.ConnectToAP(&prof);

    config::network.WaitUntilConnected();
    config::network.WaitForIP();
  }
}