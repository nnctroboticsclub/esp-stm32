#include <wifi.hpp>
#include <wifi/authmode_kind.hpp>

#include <cstring>
#include <lwip/netif.h>

#include <esp_log.h>
#include <esp_wpa2.h>
#include "init/init.hpp"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_GOT_IP BIT2
namespace {
constexpr const char* TAG = "Wi-Fi Lib Functions";

}  // namespace

namespace wifi {

void Wifi::event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data) {
  Wifi& that = *static_cast<Wifi*>(arg);
  ESP_LOGI(TAG, "[Evt] base: %s, id: %lu", event_base, event_id);
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    esp_wifi_connect();
    xEventGroupClearBits(that.s_wifi_event_group, WIFI_CONNECTED_BIT);
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    xEventGroupSetBits(that.s_wifi_event_group, WIFI_CONNECTED_BIT);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    auto event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(TAG, "Connected to Wi-Fi AP");
    ESP_LOGI(TAG, "  - IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(that.s_wifi_event_group, WIFI_GOT_IP);

  } else {
    ESP_LOGW(TAG, "UNEXPECTED EVENT base: %s, id: %lu", event_base, event_id);
  }
}

Wifi::Wifi() : s_wifi_event_group(nullptr), netif(nullptr) {
  this->s_wifi_event_group = xEventGroupCreate();
}

Wifi::~Wifi() {
  vEventGroupDelete(this->s_wifi_event_group);
  esp_wifi_stop();
  esp_wifi_deinit();
}

void Wifi::SetIP(const esp_netif_ip_info_t& ip_info) {
  if (this->netif == nullptr) {
    ESP_LOGE(TAG, "Cannot set IP address: netif is null");
    return;
  }

  esp_netif_dhcp_status_t status;
  ESP_ERROR_CHECK(esp_netif_dhcpc_get_status(this->netif, &status));

  if (status != esp_netif_dhcp_status_t::ESP_NETIF_DHCP_STOPPED) {
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(this->netif));
  }
  ESP_ERROR_CHECK(esp_netif_set_ip_info(this->netif, &ip_info));
}

void Wifi::InitAp(const char* ssid, const char* password) {
  static bool initialized = false;
  if (initialized) {
    ESP_LOGW(TAG, "Wi-Fi AP already initialized");
    return;
  }
  initialized = true;

  ESP_LOGI(TAG, "[Netif] New AP");
  wifi::init::init_netif();
  this->netif = esp_netif_create_default_wifi_ap();

  wifi::init::init_wifi_lib(&Wifi::event_handler, (void*)this);

  wifi_config_t wifi_config;
  strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
  strncpy((char*)wifi_config.ap.password, password,
          sizeof(wifi_config.ap.password));
  wifi_config.ap.ssid_len = strlen(ssid);
  wifi_config.ap.channel = 11;
  wifi_config.ap.authmode = WIFI_AUTH_WPA2_WPA3_PSK;
  wifi_config.ap.max_connection = 11;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
}

void Wifi::InitSta() {
  static bool initialized = false;
  if (initialized) {
    ESP_LOGW(TAG, "Wi-Fi STA already initialized");
    return;
  }
  initialized = true;

  ESP_LOGI(TAG, "[Netif] New Sta");
  wifi::init::init_netif();
  this->netif = esp_netif_create_default_wifi_sta();

  wifi::init::init_wifi_lib(&Wifi::event_handler, (void*)this);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
}

void Wifi::ConnectToAP(WifiConnectionProfile* prof) {
  wifi_config_t wifi_config{};
  strncpy((char*)wifi_config.sta.ssid, prof->ssid,
          sizeof(wifi_config.sta.ssid));

  wifi_config.sta.threshold.authmode = prof->auth_mode;
  wifi_config.sta.pmf_cfg.capable = true;
  wifi_config.sta.pmf_cfg.required = false;

  switch (GetAuthModeKind(prof->auth_mode)) {
    case AuthModeKind::kOpen:
      break;

    case AuthModeKind::kPassOnly:
      strncpy((char*)wifi_config.sta.password, prof->password,
              sizeof(wifi_config.sta.password));
      break;
    case AuthModeKind::kPassAndUser:
      esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)prof->id, strlen(prof->id));
      esp_wifi_sta_wpa2_ent_set_username((uint8_t*)prof->user,
                                         strlen(prof->user));
      esp_wifi_sta_wpa2_ent_set_password((uint8_t*)prof->password,
                                         strlen(prof->password));
      break;
    default:
      ESP_LOGE(TAG, "Unknown auth mode: %d", prof->auth_mode);
      exit(1);
  }

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
}

void Wifi::WaitUntilConnected() {
  ESP_LOGI(TAG, "Waiting for connection...");
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
void Wifi::WaitForIP() {
  ESP_LOGI(TAG, "Waiting for IP...");
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_GOT_IP,
                                         pdFALSE, pdFALSE, portMAX_DELAY);
  ESP_LOGI(TAG, "Got IP");
}
}  // namespace wifi

std::string std::to_string(wifi_auth_mode_t authmode) {
  switch (authmode) {
    case WIFI_AUTH_OPEN:
      return "WIFI_AUTH_OPEN";
    case WIFI_AUTH_WEP:
      return "WIFI_AUTH_WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WIFI_AUTH_WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WIFI_AUTH_WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WIFI_AUTH_WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WIFI_AUTH_WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
      return "WIFI_AUTH_WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WIFI_AUTH_WPA2_WPA3_PSK";
    case WIFI_AUTH_WAPI_PSK:
      return "WIFI_AUTH_WAPI_PSK";
    case WIFI_AUTH_OWE:
      return "WIFI_AUTH_OWE";
    case WIFI_AUTH_MAX:
      return "WIFI_AUTH_MAX";
    default:
      return "Unknown";
  }
}