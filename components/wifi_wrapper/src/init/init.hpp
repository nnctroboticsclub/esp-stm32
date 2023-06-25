#pragma once
#include <esp_event.h>

namespace wifi::init {
extern const char* TAG;
void init_netif();
void init_nvs();
void init_eventloop();
void init_wifi_lib(esp_event_handler_t event_handler, void* arg);
}  // namespace wifi::init
