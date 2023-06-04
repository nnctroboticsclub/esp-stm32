#include "wifi.hpp"
#include <cstring>
#include <lwip/netif.h>

#include <esp_log.h>

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
    ESP_LOGI(TAG, "Connected to Wi-Fi AP");
    ESP_LOGI(TAG, "  - IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(that.s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
void Wifi::EventLoopTask(void* pvWifi) {
  Wifi& wifi = *reinterpret_cast<Wifi*>(pvWifi);

  while (true) {
    auto a = xEventGroupWaitBits(wifi.s_wifi_event_group, 0x00ffffff, pdFALSE,
                                 pdTRUE, portMAX_DELAY);
    ESP_LOGD(TAG, "EventLoopTask: bits = %#08lx", a);
  }
}

Wifi::Wifi() : ssid(""), password(""), is_ap_mode(true) {}

Wifi::Wifi(const char* ssid, const char* password)
    : ssid(ssid), password(password), is_ap_mode(false) {}

Wifi::~Wifi() {
  vEventGroupDelete(this->s_wifi_event_group);
  esp_wifi_stop();
  esp_wifi_deinit();
}

void Wifi::Init() {
  this->s_wifi_event_group = xEventGroupCreate();

  if (this->is_ap_mode) {
    esp_netif_create_default_wifi_ap();
  } else {
    esp_netif_create_default_wifi_sta();

    auto netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_dhcpc_stop(netif);

    esp_netif_ip_info_t ip_info;

    ip_info.ip.addr = ipaddr_addr("172.16.34.90");
    ip_info.gw.addr = ipaddr_addr("172.16.34.254");
    ip_info.netmask.addr = ipaddr_addr("255.255.255.00");

    esp_netif_set_ip_info(netif, &ip_info);
  }

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

void Wifi::ConnectToAP() {
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = "",
              .password = "",
              .threshold =
                  {
                      .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                  },

              .pmf_cfg = {.capable = true, .required = false},
          },
  };
  strncpy((char*)wifi_config.sta.ssid, this->ssid,
          sizeof(wifi_config.sta.ssid));
  strncpy((char*)wifi_config.sta.password, this->password,
          sizeof(wifi_config.sta.password));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
void Wifi::ConnectMake() {
  wifi_config_t wifi_config = {
      .ap =
          {
              .ssid = "ESP32",
              .password = "APaPapAp",
              .ssid_len = strlen("ESP32"),
              .channel = 11,
              .authmode = WIFI_AUTH_WPA_WPA2_PSK,
              .max_connection = 11,
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void Wifi::Connect() {
  if (this->is_ap_mode) {
    this->ConnectMake();
  } else {
    this->ConnectToAP();
  }
}

void Wifi::WaitConnection() {
  if (this->is_ap_mode) {
    return;
  }
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Connected to AP");
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGE(TAG, "Failed to connect to AP");
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT bits: %#08lx", bits);
    exit(1);
  }
}
void Wifi::StartEventLoop() {
  TaskHandle_t handle;
  xTaskCreate(Wifi::EventLoopTask, "Wifi Eventloop", 0x10000,
              reinterpret_cast<void*>(this), 1, &handle);
}
}  // namespace app