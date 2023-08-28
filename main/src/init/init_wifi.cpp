#include <esp_log.h>
#include <nvs_flash.h>

#include "init.hpp"
#include "../config/config.hpp"

void init::init_wifi() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  auto profile = config::Config::GetActiveNetworkProfile();

  ESP_LOGI(TAG, "Initializing WiFi");
  ESP_LOGI(TAG, "SSID: %s, password: %s", profile->GetSSID().c_str(),
           profile->GetPassword().c_str());

  if (profile->GetMode() == types::NetworkMode::AP) {
    config::network.InitAp(profile->GetSSID(), profile->GetPassword());
  } else {
    wifi::WifiConnectionProfile prof{
        .auth_mode = WIFI_AUTH_WPA2_PSK,
        .ssid = profile->GetSSID(),
        .password = profile->GetPassword(),
        .user = "",
        .id = "",
    };
    config::network.InitSta();
    config::network.ConnectToAP(&prof);

    config::network.WaitUntilConnected();

    if (profile->GetIPMode() == types::IPMode::STATIC) {
      esp_netif_ip_info_t ip_info{
          .ip = types::ToEspIp4Addr((types::Ipv4)profile->ip),
          .netmask = types::ToEspIp4Addr((types::Ipv4)profile->subnet),
          .gw = types::ToEspIp4Addr((types::Ipv4)profile->gateway),
      };
      config::network.SetIP(ip_info);
    } else {
      config::network.WaitForIP();
    }
  }
}