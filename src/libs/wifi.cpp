#include "wifi.hpp"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

namespace app {
void Wifi::event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data) {
  Wifi& that = *static_cast<Wifi*>(arg);

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    esp_wifi_connect();
    xEventGroupClearBits(that.s_wifi_event_group, WIFI_CONNECTED_BIT);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    printf("I: [Wi-Fi] Connected to Wifi\n");
    printf("D: [Wi-Fi] - Got IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(that.s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
void Wifi::EventLoopTask(void* pvWifi) {
  Wifi& wifi = *reinterpret_cast<Wifi*>(pvWifi);

  while (true) {
    auto a = xEventGroupWaitBits(wifi.s_wifi_event_group, 0x00ffffff, pdFALSE,
                                 pdTRUE, portMAX_DELAY);
    printf("V: [Wi-Fi] WaitBits returns %ld\n", a);
  }
}

Wifi::Wifi() {}

Wifi::~Wifi() {
  vEventGroupDelete(this->s_wifi_event_group);
  esp_wifi_stop();
  esp_wifi_deinit();
}

void Wifi::Init() {
  this->s_wifi_event_group = xEventGroupCreate();

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void Wifi::Setup() {
  esp_event_handler_instance_t instance_any_id;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::event_handler, (void*)this,
      &instance_any_id));

  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi::event_handler, (void*)this,
      &instance_got_ip));
}

void Wifi::Connect() {
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = "***REMOVED***",
              .password = "***REMOVED***",
              .threshold =
                  {
                      .authmode = WIFI_AUTH_WPA2_PSK,
                  },

              .pmf_cfg = {.capable = true, .required = false},
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void Wifi::WaitConnection() {
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & WIFI_CONNECTED_BIT) {
    printf("I: [Wi-Fi] connected to AP \n");
  } else if (bits & WIFI_FAIL_BIT) {
    printf("E: [Wi-Fi] Failed to connect to Wifi\n");
  } else {
    printf("F: [Wi-Fi] UNEXPECTED EVENT (bits = %ld)\n", bits);
    exit(1);
  }
}
void Wifi::StartEventLoop() {
  TaskHandle_t handle;
  xTaskCreate(Wifi::EventLoopTask, "Wifi Eventloop", 0x10000,
              reinterpret_cast<void*>(this), 1, &handle);
}
}  // namespace app