#pragma once
#ifndef WIFI_HPP
#define WIFI_HPP

#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <inttypes.h>
#include <freertos/event_groups.h>
#include <esp_event_base.h>

namespace app {
class Wifi {
  static constexpr const char* TAG = "Wi-Fi";
  const char* ssid;
  const char* password;

  esp_ip4_addr_t ip;
  esp_ip4_addr_t gw;
  esp_ip4_addr_t mask;

  EventGroupHandle_t s_wifi_event_group;

  static void event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
  static void EventLoopTask(void* pvWifi);

  void InitNetif();

 public:
  Wifi();
  ~Wifi();

  void SetCredentials(const char* ssid, const char* password);
  void SetIP(const esp_ip4_addr_t& ip, const esp_ip4_addr_t& gw,
             const esp_ip4_addr_t& mask);

  void InitAp();
  void InitSta();

  void WaitConnection();  // For sta mode!
};
}  // namespace app

#endif