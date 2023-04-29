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
  EventGroupHandle_t s_wifi_event_group;

  static void event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
  static void EventLoopTask(void* pvWifi);

 public:
  Wifi();
  ~Wifi();
  void Init();
  void Setup();
  void Connect();

  void WaitConnection();
  void StartEventLoop();
};
}  // namespace app

#endif