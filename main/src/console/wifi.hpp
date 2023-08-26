#pragma once

#include "common.hpp"
#include <esp_wifi.h>
#include <esp_log.h>
#include "../config/config.hpp"

namespace cmd::wifi {

wifi_ap_record_t* ap_list = nullptr;
uint16_t ap_num = 0;

class ScanCommand {
 public:
  static constexpr const char* TAG = "Wifi";
  static constexpr const char* kCommandName = "wifi-scan";
  static constexpr const char* kHelp = "Wifi Scan";

  static struct scan_args {
   public:
    struct arg_end* end;
    class ScanCommand* obj;
  } args;

  static int CmdLineHandler(int argc, char** argv) {
    auto errors = arg_parse(argc, argv, (void**)&ScanCommand::args);
    if (errors != 0) {
      arg_print_errors(stderr, ScanCommand::args.end, argv[0]);
      return 1;
    }

    return ScanCommand::args.obj->Handler();
  }

  void InitArgs() { args.end = arg_end(10); }
  int Handler() {
    config::network.Stop();
    config::network.InitSta();
    config::network.Start();
    if (ap_list) {
      free(ap_list);
    }

    ESP_LOGI(TAG, "Scaning...");

    esp_wifi_scan_start(NULL, true);

    esp_wifi_scan_get_ap_num(&ap_num);

    ap_list = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * ap_num);
    esp_wifi_scan_get_ap_records(&ap_num, ap_list);

    for (int i = 0; i < ap_num; i++) {
      printf("[%3d] %20s {rssi: %d} auth: %s\n", i, ap_list[i].ssid,
             ap_list[i].rssi, std::to_string(ap_list[i].authmode).c_str());
    }

    esp_wifi_scan_stop();

    return 0;
  }
};

ScanCommand::scan_args ScanCommand::args = {};

class ConnectCommand {
 public:
  static constexpr const char* TAG = "Wifi-Connect";
  static constexpr const char* kCommandName = "wifi-connect";
  static constexpr const char* kHelp = "Wifi Connect to Specified AP";

  static struct Args {
   public:
    struct arg_int* idx;
    struct arg_end* end;
    class ConnectCommand* obj;
  } args;

  static int CmdLineHandler(int argc, char** argv) {
    auto errors = arg_parse(argc, argv, (void**)&ConnectCommand::args);
    if (errors != 0) {
      arg_print_errors(stderr, ConnectCommand::args.end, argv[0]);
      return 1;
    }

    return ConnectCommand::args.obj->Handler();
  }

  void InitArgs() {
    args.idx = arg_int1(NULL, NULL, "<n>", "AP Index");
    args.end = arg_end(10);
  }
  int Handler() {
    if (!ap_list) {
      ESP_LOGE(TAG, "AP List is not scanned yet");
      return 1;
    }

    auto idx = args.idx->ival[0];
    if (idx < 0 || idx >= ap_num) {
      ESP_LOGE(TAG, "Invalid index: %d", idx);
      return 1;
    }

    auto& ap = ap_list[idx];
    auto auth_mode = ::wifi::GetAuthModeKind(ap.authmode);

    ::wifi::WifiConnectionProfile profile{
        .auth_mode = ap.authmode,
        .ssid = (const char*)ap.ssid,
        .password = "",
        .user = "",
        .id = "",
    };

    if (auth_mode == ::wifi::AuthModeKind::kOpen) {
      // Do nothing
    } else if (auth_mode == ::wifi::AuthModeKind::kPassOnly) {
      std::string password;
      printf("Password: ");
      std::cin >> password;
      profile.password = password;

      auto p = new char[password.length() + 1];
      memcpy(p, password.c_str(), password.length() + 1);
      profile.password = p;
    } else if (auth_mode == ::wifi::AuthModeKind::kPassAndUser) {
      std::string identify;
      std::string username;
      std::string password;

      printf("Identify: ");
      std::cin >> identify;

      printf("Username: ");
      std::cin >> username;

      printf("Password: ");
      std::cin >> password;

      auto p = new char[identify.length() + 1];
      memcpy(p, identify.c_str(), identify.length() + 1);
      profile.id = std::string(p);

      p = new char[username.length() + 1];
      memcpy(p, username.c_str(), username.length() + 1);
      profile.user = std::string(p);

      p = new char[password.length() + 1];
      memcpy(p, password.c_str(), password.length() + 1);
      profile.password = std::string(p);
    }

    config::network.Stop();
    config::network.InitSta();
    config::network.Start();
    config::network.ConnectToAP(&profile);
    config::network.WaitUntilConnected();

    return 0;
  }
};

ConnectCommand::Args ConnectCommand::args = {};

void RegisterCommands() {
  ScanCommand sc{};
  RegisterCommand(sc);

  ConnectCommand cc{};
  RegisterCommand(cc);
}

}  // namespace cmd::wifi