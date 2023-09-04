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

#include "./http_server.hpp"
#include "config/config.hpp"
#include "init/init.hpp"
#include "libs/gpio.hpp"
#include "libs/button.hpp"
#include "console/wifi.hpp"
#include "conn_proxy/conn_proxy.hpp"

const char* const TAG = "Main";

void BootStrap() {
  using Config = config::Config;

  ESP_LOGI(TAG, "Booting up (stage1: BootStrap)");
  nvs::DumpNVS();

  idf::GPIOInput flag(idf::GPIONum(22));
  flag.set_pull_mode(idf::GPIOPullMode::PULLDOWN());
  if (flag.get_level() == idf::GPIOLevel::HIGH) {
    ESP_LOGI(TAG, "Erasing the NVS (reason: gpio 22 is high)");
    nvs_flash_erase();
  }
}

void Init() {
  using Config = config::Config;

  ESP_LOGI(TAG, "Booting up (stage2: Init)");

  auto& config = Config::GetInstance();

  if (!config.master.initialised) {
    ESP_LOGI(TAG, "First boot, initialising the nvs");
    config.master.initialised = true;

    Config::NewSPIBus()
        .SetSpiPort(2)
        .SetMiso(GPIO_NUM_19)
        .SetMosi(GPIO_NUM_23)
        .SetSclk(GPIO_NUM_18)
        .Commit();

    Config::NewSTM32BL()
        .SetID(1)
        .SetBusType(types::BusType::SPI)
        .SetBusPort(2)
        .SetCS(5)
        .Commit();

    Config::NewSTM32()
        .SetID(2)
        .SetReset(GPIO_NUM_21)
        .SetBoot0(GPIO_NUM_22)
        .SetBL_ID(1)
        .SetRC_ID(0)
        .Commit();

    Config::NewNetworkProfile()
        .SetID(3)
        .SetMode(types::NetworkMode::AP)
        .SetIPMode(types::IPMode::STATIC)
        .SetSSID("ESP32")
        .SetPassword("esp32-network")
        .SetHostname("esp32")
        .SetIP((types::Ipv4)0xc0a80001)
        .SetSubnet((types::Ipv4)0xffffff00)
        .SetGateway((types::Ipv4)0xc0a80001);

    Config::SetActiveSTM32(2);
    Config::SetActiveNetworkProfile(3);

    Config::GetInstance().master.Commit();
  }
  return;
}

debug_httpd::DebuggerHTTPServer server;

void Main() {
  ESP_LOGI(TAG, "Starting Debugger HTTP Server");
  server.Listen(80);
}

extern "C" int app_main() {
  BootStrap();

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

  Main();

  printf("Entering the idle loop\n");
  while (true) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  return 0;
}