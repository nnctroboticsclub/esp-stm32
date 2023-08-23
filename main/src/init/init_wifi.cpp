#include "init.hpp"

#include "../config/config.hpp"

#include <esp_log.h>
#include <nvs_flash.h>

void init::init_wifi() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  auto profile = config::Config::GetActiveNetworkProfile();

  ESP_LOGI(TAG, "Initializing WiFi [%s]", profile->name.Get().c_str());
  ESP_LOGI(TAG, "SSID: %s, password: %s", profile->ssid.Get().c_str(),
           profile->password.Get().c_str());
  if (profile->ip_mode.Get() == types::IPMode::STATIC) {
    esp_netif_ip_info_t ip_info{
        .ip = types::ToEspIp4Addr((types::Ipv4)profile->ip),
        .netmask = types::ToEspIp4Addr((types::Ipv4)profile->subnet),
        .gw = types::ToEspIp4Addr((types::Ipv4)profile->gateway),
    };
    config::network.SetIP(ip_info);
  }

  if (profile->mode.Get() == types::NetworkMode::AP) {
    config::network.InitAp(profile->ssid.Get(), profile->password.Get());

    config::network.WaitUntilConnected();
  } else {
    wifi::WifiConnectionProfile prof{
        .auth_mode = WIFI_AUTH_WPA2_PSK,
        .ssid = profile->ssid.Get(),
        .password = profile->password.Get(),
        .user = "",
        .id = "",
    };
    config::network.InitSta();
    config::network.ConnectToAP(&prof);

    config::network.WaitUntilConnected();
    config::network.WaitForIP();
  }
}