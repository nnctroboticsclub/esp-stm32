#include <wifi.hpp>
#include <cstring>
#include <lwip/netif.h>

#include <esp_log.h>
#include "init/init.hpp"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

namespace {
constexpr const char* TAG = "Wi-Fi Lib Functions";
void SetIPToNetif(const char* netif_name, esp_ip4_addr_t ip, esp_ip4_addr_t gw,
                  esp_ip4_addr_t mask) {
  ESP_LOGI(TAG, "Setting IP to netif %s", netif_name);
  ESP_LOGI(TAG, "  - IP: " IPSTR, IP2STR(&ip));
  ESP_LOGI(TAG, "  - GW: " IPSTR, IP2STR(&gw));
  ESP_LOGI(TAG, "  - MASK: " IPSTR, IP2STR(&mask));
  esp_netif_t* netif = esp_netif_get_handle_from_ifkey(netif_name);
  esp_netif_dhcpc_stop(netif);
  esp_netif_ip_info_t ip_info{
      .ip = ip,
      .netmask = mask,
      .gw = gw,
  };

  esp_netif_set_ip_info(netif, &ip_info);
}
}  // namespace

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
  } else {
    ESP_LOGI(TAG, "UNEXPECTED EVENT base: %s, id: %lu", event_base, event_id);
  }
}

Wifi::Wifi()
    : ssid(nullptr),
      password(nullptr),
      ip{0},
      gw{0},
      mask{0},
      s_wifi_event_group(nullptr) {}

Wifi::~Wifi() {
  vEventGroupDelete(this->s_wifi_event_group);
  esp_wifi_stop();
  esp_wifi_deinit();
}
void Wifi::InitHW() {
  wifi_init::init_nvs();
  ESP_LOGI(TAG, "Creating Wi-Fi event group");
  this->s_wifi_event_group = xEventGroupCreate();

  wifi_init::init_netif();
  wifi_init::init_eventloop();
}

void Wifi::SetCredentials(const char* ssid, const char* password) {
  this->ssid = ssid;
  this->password = password;
}

void Wifi::SetIP(const esp_ip4_addr_t& ip, const esp_ip4_addr_t& gw,
                 const esp_ip4_addr_t& mask) {
  this->ip = ip;
  this->gw = gw;
  this->mask = mask;
}

void Wifi::InitAp() {
  esp_netif_t* netif;
  esp_netif_create_default_wifi_ap();
  SetIPToNetif("WIFI_AP_DEF", this->ip, this->gw, this->mask);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::event_handler, (void*)this, NULL));

  wifi_config_t wifi_config{};
  strncpy((char*)wifi_config.ap.ssid, this->ssid, sizeof(wifi_config.ap.ssid));
  strncpy((char*)wifi_config.ap.password, this->password,
          sizeof(wifi_config.ap.password));
  wifi_config.ap.ssid_len = strlen(this->ssid);
  wifi_config.ap.channel = 11;
  wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  wifi_config.ap.max_connection = 11;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
void Wifi::InitSta() {
  esp_netif_t* netif;
  esp_netif_create_default_wifi_sta();

  SetIPToNetif("WIFI_STA_DEF", this->ip, this->gw, this->mask);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t dummy;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::event_handler, (void*)this, &dummy));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, ESP_EVENT_ANY_ID, &Wifi::event_handler, (void*)this, &dummy));

  wifi_config_t wifi_config{};
  strncpy((char*)wifi_config.sta.ssid, this->ssid,
          sizeof(wifi_config.sta.ssid));
  strncpy((char*)wifi_config.sta.password, this->password,
          sizeof(wifi_config.sta.password));
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  wifi_config.sta.pmf_cfg.capable = true;
  wifi_config.sta.pmf_cfg.required = false;
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void Wifi::WaitConnection() {
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
}  // namespace app