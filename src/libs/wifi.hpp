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
  EventGroupHandle_t s_wifi_event_group;

  static void event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
  static void EventLoopTask(void* pvWifi);

  bool is_ap_mode;

  void ConnectToAP();
  void ConnectMake();

 public:
  Wifi();
  Wifi(const char* ssid, const char* password);
  ~Wifi();
  void Init();
  void Setup();
  void Connect();

  void WaitConnection();
  void StartEventLoop();

  bool IsAPMode();
};
}  // namespace app

#endif