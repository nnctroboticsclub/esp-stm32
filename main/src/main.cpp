#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs.h>
#include <esp_system.h>
#include <esp_console.h>

#include <thread>

#include <stm32.hpp>
#include <stm32/raw_driver/impl/uart.hpp>
#include <spi_host_cxx.hpp>

#include "./bin.h"

#include "./http_server.hpp"
#include "config/config.hpp"
#include "init/init.hpp"
#include "libs/gpio.hpp"
#include "libs/button.hpp"
#include "console/wifi.hpp"

const char* const TAG = "Main";

void BootStrap() {
  using Config = config::Config;

  idf::GPIOInput flag(idf::GPIONum(22));
  flag.set_pull_mode(idf::GPIOPullMode::PULLDOWN());
  if (flag.get_level() == idf::GPIOLevel::HIGH) {
    ESP_LOGI(TAG, "Erasing the NVS (reason: gpio 22 is high)");
    nvs_flash_erase();
  }

  auto& config = Config::GetInstance();

  if (!config.master.initialised) {
    ESP_LOGI(TAG, "First boot, initialising the nvs");
    config.master.initialised = true;

    Config::NewSPIBus()
        .SetSpiPort(2)
        .SetMiso(GPIO_NUM_19)
        .SetMosi(GPIO_NUM_23)
        .SetSclk(GPIO_NUM_18);

    Config::NewSTM32BL()
        .SetID(0)
        .SetBusType(types::BusType::SPI)
        .SetBusPort(2)
        .SetCS(0);

    Config::NewSTM32()
        .SetID(0)
        .SetReset(GPIO_NUM_21)
        .SetBoot0(GPIO_NUM_22)
        .SetBL_ID(0)
        .SetRC_ID(0);

    Config::SetActiveSTM32(0);
  }
}

void Init() {
  // init::init_data_server();
  return;

  // auto config = config::Config::GetInstance();
  // xTaskCreate((TaskFunction_t)([](void* args) {
  //               while (true) {
  //                 vTaskDelay(50 / portTICK_PERIOD_MS);
  //                 ((DebuggerMaster*)args)->Idle();
  //               }
  //               return;
  //             }),
  //             "Debugger Idling Thread", 0x1000,
  //             config->stm32_remote_controller_profile.GetDebuggerMaster(), 1,
  //             nullptr);
}

void Main() {
  ESP_LOGI(TAG, "Starting Debugger HTTP Server");
  DebuggerHTTPServer server(config::Config::GetPrimarySTM32());
  server.Listen();

  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();
}

extern "C" int app_main() {
  wifi::WifiConnectionProfile profile{
      .auth_mode = WIFI_AUTH_WPA_WPA2_PSK,
      .ssid = "3-303-Abe 2.4Ghz",
      .password = "syochnetwork",
      .user = "",
      .id = "",
  };
  config::network.InitSta();
  config::network.Start();
  config::network.ConnectToAP(&profile);
  config::network.WaitUntilConnected();
  config::network.WaitForIP();

  Init();

  std::jthread t([]() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_console_repl_t* repl = nullptr;
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

  // t.join();

  Main();
  printf("Entering the idle loop\n");
  while (true) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  return 0;
}