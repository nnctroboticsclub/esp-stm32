#include <esp_log.h>

#include "config/config.hpp"
#include "init/init.hpp"
#include "bin.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <esp_system.h>

#include <esp_console.h>
#include <spi_host_cxx.hpp>
#include "libs/gpio.hpp"
const char* TAG = "Main";

#include <stm32bl/stm32bl_spi.hpp>

class UserButton {
  static constexpr const char* TAG = "UserButton";

 private:
  gpio_num_t pin;

 public:
  UserButton(gpio_num_t button) : pin(button) {
    ESP_LOGI(TAG, "Setting GPIO %d as input", button);
    gpio_set_direction(button, GPIO_MODE_INPUT);
  }

  bool IsPressed() { return gpio_get_level(this->pin) == 1; }

  void WaitUntilPressed() {
    while (!this->IsPressed()) vTaskDelay(50 / portTICK_PERIOD_MS);

    while (this->IsPressed()) vTaskDelay(50 / portTICK_PERIOD_MS);
  }
};

void BootStrap() {
  UserButton btn(GPIO_NUM_22);
  if (btn.IsPressed()) {
    ESP_LOGI(TAG, "Erasing the NVS (reason: Button 22 is pressed = 1 (high))");
    nvs_flash_erase();
  }

  idf::SPIMaster master(idf::SPINum(2), idf::MOSI(23), idf::MISO(19),
                        idf::SCLK(18));

  nvs::SharedNamespace flags("a_flags");
  nvs::Proxy<bool> initialised{flags, "initialized"};
  if (!initialised) {
    ESP_LOGI(TAG, "First boot, initialising the nvs");
    initialised = true;

    {
      nvs::SharedNamespace stm32bl_ns = ("a_s32bl");

      auto stm32bl = profile::SpiSTM32BootLoaderProfile(stm32bl_ns);

      stm32bl.reset = GPIO_NUM_21;
      stm32bl.boot0 = GPIO_NUM_22;
      stm32bl.cs = GPIO_NUM_5;
      stm32bl.spi_port = 2;

      stm32bl.Save();
    }

    {
      auto config = config::Config::GetInstance();
      config->server_profile.ip = (types::Ipv4){.ip_bytes = {192, 168, 11, 7}};
      config->server_profile.port = 8080;
      config->server_profile.Save();
      config->network_profiles[0].mode = types::NetworkMode::STA;
      config->network_profiles[0].name = "Network@Ryo";
      config->network_profiles[0].ssid = "***REMOVED***";
      config->network_profiles[0].password = "***REMOVED***";
      config->network_profiles[0].hostname = "esp32-0610";
      config->network_profiles[0].ip_mode = types::IPMode::DHCP;
      config->network_profiles[0].ip = 0;
      config->network_profiles[0].subnet = 0;
      config->network_profiles[0].gateway = 0;
      config->network_profiles[0].Save();
      config->network_profiles[1].mode = types::NetworkMode::STA;
      config->network_profiles[1].name = "Tethering";
      config->network_profiles[1].ssid = "***REMOVED***";
      config->network_profiles[1].password = "***REMOVED***";
      config->network_profiles[1].hostname = "esp32-0610";
      config->network_profiles[1].ip_mode = types::IPMode::DHCP;
      config->network_profiles[1].ip = 0;
      config->network_profiles[1].subnet = 0;
      config->network_profiles[1].gateway = 0;
      config->network_profiles[1].Save();

      config->network_profiles[2].mode = types::NetworkMode::AP;
      config->network_profiles[2].name = "AP (ESP32-syoch)";
      config->network_profiles[2].ssid = "ESP32-syoch";
      config->network_profiles[2].password = "esp32-0610";
      config->network_profiles[2].hostname = "esp32-0610";
      config->network_profiles[2].ip_mode = types::IPMode::STATIC;
      config->network_profiles[2].ip =
          types::Ipv4{.ip_bytes = {192, 168, 1, 1}};
      config->network_profiles[2].subnet =
          types::Ipv4{.ip_bytes = {255, 255, 255, 0}};
      config->network_profiles[2].gateway =
          types::Ipv4{.ip_bytes = {192, 168, 1, 1}};
      config->network_profiles[2].Save();
      config->stm32_remote_controller_profile.uart_port = 1;
      config->stm32_remote_controller_profile.uart_tx = 17;
      config->stm32_remote_controller_profile.uart_rx = 16;
      config->stm32_remote_controller_profile.Save();
      config->active_network_profile = 0;
      config->active_network_profile.Commit();
    }

    flags->Commit();
  }

  auto config = config::Config::GetInstance();

  config->active_network_profile = 1;
}
void Init() {
  auto config = config::Config::GetInstance();
  //   init::init_data_server();
  //
  //   xTaskCreate((TaskFunction_t)([](void* args) {
  //                 while (1) {
  //                   vTaskDelay(50 / portTICK_PERIOD_MS);
  //                   ((DebuggerMaster*)args)->Idle();
  //                 }
  //                 return;
  //               }),
  //               "Debugger Idling Thread", 0x1000,
  //               config->stm32_remote_controller_profile.GetDebuggerMaster(),
  //               1, nullptr);
}

TaskResult Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();

  // stm32bl::Stm32BootLoaderSPI bl(GPIO_NUM_27, GPIO_NUM_26, master, 5);
  //
  // RUN_TASK_V(bl.Connect());
  // RUN_TASK_V(bl.Get());
  //
  // bl.Erase(0x08000000, new_flash_len);
  // RUN_TASK_V(bl.WriteMemory(0x08000000, new_flash, new_flash_len));

  /*
  nvs_iterator_t it;
  auto ret = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL,
                            nvs_type_t::NVS_TYPE_ANY, &it);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Error ocurred while finding nvs entries: %s",
             esp_err_to_name(ret));

    return;
  }

  ESP_LOGI(TAG, "First it = %p", it);
  while (it) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);

    nvs_handle_t handle;
    nvs_open(info.namespace_name, NVS_READONLY, &handle);
    switch (info.type) {
      case NVS_TYPE_U8: {
        uint8_t value;
        nvs_get_u8(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (u8)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_U16: {
        uint16_t value;
        nvs_get_u16(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (u16)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_U32: {
        uint32_t value;
        nvs_get_u32(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %lu (u32)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_U64: {
        uint64_t value;
        nvs_get_u64(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %llu (u64)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I8: {
        int8_t value;
        nvs_get_i8(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (s8)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I16: {
        int16_t value;
        nvs_get_i16(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (s16)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I32: {
        int32_t value;
        nvs_get_i32(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %ld (s32)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I64: {
        int64_t value;
        nvs_get_i64(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %lld (s64)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_STR: {
        size_t length = 0;
        nvs_get_str(handle, info.key, NULL, &length);

        char* value = (char*)malloc(length);
        nvs_get_str(handle, info.key, value, &length);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %s (str)", info.namespace_name,
                 info.key, value);

        free(value);
        break;
      }

      case NVS_TYPE_BLOB: {
        size_t length = 0;
        nvs_get_blob(handle, info.key, NULL, &length);

        uint8_t* value = (uint8_t*)malloc(length);
        nvs_get_blob(handle, info.key, (void*)value, &length);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = <blob>", info.namespace_name,
                 info.key);

        printf("  ");
        for (size_t i = 0; i < length; i++) {
          printf("%02x ", value[i]);
          if (i % 16 == 15) {
            printf("\n  ");
          }
        }

        free(value);
        break;
      }
      case NVS_TYPE_ANY:
      default:
        ESP_LOGW(TAG, "NVS Entry: %s::%s = <unknown>", info.namespace_name,
                 info.key);
        break;
    }
    nvs_entry_next(&it);
    nvs_close(handle);
  }
  */

  return TaskResult::Ok();
}
#include <argtable3/argtable3.h>
#include <functional>
#include <wifi/authmode_kind.hpp>
#include <iostream>

namespace cmd {
template <typename T, typename Class>
concept CommandArgs = requires(T a) {
  { a.end } -> std::convertible_to<struct arg_end*>;
  { a.obj } -> std::convertible_to<Class*>;
};

template <typename T>
concept Command = requires(T a) {
  { T::TAG } -> std::convertible_to<const char*>;
  { T::kCommandName } -> std::convertible_to<const char*>;
  { T::kHelp } -> std::convertible_to<const char*>;
  { T::args } -> CommandArgs<T>;

  { T::CmdLineHandler(0, (char**)nullptr) } -> std::same_as<int>;

  { a.InitArgs() } -> std::same_as<void>;
  { a.Handler() } -> std::same_as<int>;
};

template <Command Cmd>
void RegisterCommand(Cmd& command) {
  command.InitArgs();
  command.args.obj = &command;
  esp_console_cmd_t cmd_conf = {.command = Cmd::kCommandName,
                                .help = Cmd::kHelp,
                                .hint = NULL,
                                .func = &Cmd::CmdLineHandler,
                                .argtable = &Cmd::args};
  esp_console_cmd_register(&cmd_conf);
}
namespace wifi {

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
    config::network.InitSta();
    ESP_ERROR_CHECK(esp_wifi_start());
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
      profile.password = password.c_str();

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
      profile.id = p;

      p = new char[username.length() + 1];
      memcpy(p, username.c_str(), username.length() + 1);
      profile.user = p;

      p = new char[password.length() + 1];
      memcpy(p, password.c_str(), password.length() + 1);
      profile.password = p;
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    config::network.InitSta();
    ESP_ERROR_CHECK(esp_wifi_start());
    config::network.ConnectToAP(&profile);
    config::network.WaitUntilConnected();

    return 0;
  }
};

ConnectCommand::Args ConnectCommand::args = {};
}  // namespace wifi

void StartConsole() {
  esp_console_repl_t* repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = "esp32-ru> ";

  esp_console_register_help_command();

  wifi::ScanCommand sc{};
  wifi::ConnectCommand cc{};
  RegisterCommand(sc);
  RegisterCommand(cc);

  esp_console_dev_uart_config_t uart_config =
      ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
}  // namespace cmd

extern "C" int app_main() {
  BootStrap();

  cmd::StartConsole();

  return 0;
  Init();
  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}