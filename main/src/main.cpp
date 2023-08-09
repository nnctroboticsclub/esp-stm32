#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs.h>
#include <esp_system.h>
#include <esp_console.h>

#include <thread>

#include <stm32bl/stm32bl_spi.hpp>
#include <stm32bl/stm32bl_uart.hpp>
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
  idf::GPIOInput flag(idf::GPIONum(22));
  flag.set_pull_mode(idf::GPIOPullMode::PULLDOWN());
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

    auto stm32bl =
        profile::SpiSTM32BootLoaderProfile(nvs::SharedNamespace("a_s32bl"));

    stm32bl.reset = GPIO_NUM_21;
    stm32bl.boot0 = GPIO_NUM_22;
    stm32bl.cs = GPIO_NUM_5;
    stm32bl.spi_port = 2;

    stm32bl.Save();

    flags->Commit();
  }
}
void Init() {
  init::init_data_server();
  return;

  auto config = config::Config::GetInstance();
  xTaskCreate((TaskFunction_t)([](void* args) {
                while (true) {
                  vTaskDelay(50 / portTICK_PERIOD_MS);
                  ((DebuggerMaster*)args)->Idle();
                }
                return;
              }),
              "Debugger Idling Thread", 0x1000,
              config->stm32_remote_controller_profile.GetDebuggerMaster(), 1,
              nullptr);
}

void Main() {
  idf::SPIMaster master{(idf::SPINum)2, (idf::MOSI)23, (idf::MISO)19,
                        (idf::SCLK)18};

  auto blp = (STM32BootLoader*)(new Stm32BootLoaderSPI{
      (idf::GPIONum)21, (idf::GPIONum)22, master, (idf::CS)5});

  std::unique_ptr<STM32BootLoader> bl(blp);

  ESP_LOGI(TAG, "Starting Debugger HTTP Server");
  DebuggerHTTPServer server(std::move(bl));
  server.Listen();

  ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  config::server.StartClientLoopAtForeground();
}

extern "C" int app_main() {
  using namespace connection::application::stm32bl;

  using connection::data_link::UART;
  using stm32::raw_driver::RawDriver;

  auto dl = std::make_shared<UART>(UART(UART_NUM_1));
  auto rd = std::make_shared<RawDriver<UART>>(RawDriver<UART>(dl));

  stm32::Session<RawDriver<UART>> session(rd, (idf::GPIONum)21,
                                          (idf::GPIONum)22);

  { auto bl_sess = session.EnterBL(); }

  BootStrap();
  wifi::WifiConnectionProfile profile{
      .auth_mode = WIFI_AUTH_WPA_WPA2_PSK,
      .ssid = "SYOCH-DESU 0136",
      .password = "j281D4<2",
      .user = "",
      .id = "",
  };
  config::network.InitSta();
  config::network.Start();
  config::network.ConnectToAP(&profile);
  config::network.WaitUntilConnected();
  config::network.WaitForIP();

  Init();

  // Stm32BootLoaderUart bl_(  //
  //     (idf::GPIONum)21,     // reset
  //     (idf::GPIONum)22,     // boot0
  //     UART_NUM_1,
  //     17,  // tx
  //     16   // rx
  // );

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

  t.join();

  Main();
  printf("Entering the idle loop\n");
  while (true) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  return 0;
}