#pragma once

#include <inttypes.h>

#include <optional>
#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <esp_wifi.h>
#include <esp_event_base.h>

#include "wifi/connection_profile.hpp"

namespace wifi {

class Wifi {
  static constexpr const char* TAG = "Wi-Fi";

  EventGroupHandle_t s_wifi_event_group;

 public:
  esp_netif_t* netif;

 private:
  static void event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);

 public:
  Wifi();
  ~Wifi();

  void SetIP(const esp_netif_ip_info_t& ip_info);

  void InitAp(const char* ssid, const char* password);
  void InitSta();

  void ConnectToAP(WifiConnectionProfile* profile);

  void WaitUntilConnected();
  void WaitForIP();
};
}  // namespace wifi

namespace std {
std::string to_string(wifi_auth_mode_t authmode);
}  // namespace std
