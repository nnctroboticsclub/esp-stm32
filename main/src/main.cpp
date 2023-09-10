#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs.h>
#include <esp_system.h>
#include <esp_console.h>

#include <thread>

#include <stm32.hpp>
#include <stm32/raw_driver/impl/uart.hpp>
#include <data_proxy/master.hpp>
#include <spi_host_cxx.hpp>

#include "./http_server.hpp"
#include "config/config.hpp"
#include "init/init.hpp"
#include "libs/gpio.hpp"
#include "libs/button.hpp"
#include "console/wifi.hpp"

#include <driver/i2c.h>
const char* const TAG = "Main";

class App {
  using Config = config::Config;
  debug_httpd::DebuggerHTTPServer server;

  void I2CDump() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.scl_io_num = 22;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 10000;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    i2c_param_config(I2C_NUM_0, &conf);

    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    esp_err_t res;
    printf("\nI2C Dump");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:         ");
    for (uint8_t i = 3; i < 0x78; i++) {
      i2c_cmd_handle_t cmd = i2c_cmd_link_create();

      ESP_ERROR_CHECK(i2c_master_start(cmd));
      ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE,
                                            1 /* expect ack */));
      ESP_ERROR_CHECK(i2c_master_stop(cmd));

      res = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);

      if (i % 16 == 0) printf("\n%.2x:", i);
      if (res == 0)
        printf(" %.2x", i);
      else
        printf(" --");
      i2c_cmd_link_delete(cmd);
    }
    printf("\n");
  }

  void BootStrap() {
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
  }

  void Main() {
    ESP_LOGI(TAG, "Starting Debugger HTTP Server");
    server.Listen(80);
  }

  [[noreturn]] void Halt() {
    printf("Entering the idle loop\n");
    while (true) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }

 public:
  [[noreturn]] void Run() {
    I2CDump();

    auto uart_dev =
        new stream::datalink::UART(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, 9600,
                                   uart_parity_t::UART_PARITY_DISABLE);
    auto dev_ = dynamic_cast<stream::RecvAndSend*>(uart_dev);
    auto dev = std::shared_ptr<stream::RecvAndSend>(dev_);
    auto link = esp_stm32::data_proxy::Link(dev);
    esp_stm32::data_proxy::Master proxy{link};

    proxy.Start().join();

    // BootStrap();
    // Init();
    // Main();

    Halt();
  }
};

extern "C" int app_main() {
  App app;
  app.Run();

  // std::jthread t([]() {
  //   vTaskDelay(1000 / portTICK_PERIOD_MS);
  //   esp_console_repl_t* repl = nullptr;
  //   esp_console_repl_config_t repl_config =
  //   ESP_CONSOLE_REPL_CONFIG_DEFAULT(); repl_config.prompt = "esp32-ru> ";
  //   esp_console_register_help_command();
  //   cmd::wifi::RegisterCommands();
  //   esp_console_dev_uart_config_t uart_config =
  //       ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  //   ESP_ERROR_CHECK(
  //       esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
  //   ESP_ERROR_CHECK(esp_console_start_repl(repl));
  // });

  return 0;
}