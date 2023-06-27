#include <esp_log.h>
#include <nvs.h>
#include <esp_system.h>
#include <esp_console.h>

#include <spi_host_cxx.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config/config.hpp"
#include "init/init.hpp"
#include "libs/gpio.hpp"
#include "libs/button.hpp"
#include "console/wifi.hpp"

const char* TAG = "Main";

void BootStrap() {
  idf::GPIOInput flag(idf::GPIONum(22));
  if (flag.get_level() == idf::GPIOLevel::HIGH) {
    ESP_LOGI(TAG, "Erasing the NVS (reason: gpio 22 is high)");
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
  init::init_data_server();
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
  ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  config::server.StartClientLoopAtForeground();

  return TaskResult::Ok();
}

extern "C" int app_main() {
  BootStrap();
  Init();

  std::thread t([]() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_console_repl_t* repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "esp32-ru> ";

    esp_console_register_help_command();

    cmd::wifi::RegisterCommands();

    esp_console_dev_uart_config_t uart_config =
        ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(
        esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
  });

  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}