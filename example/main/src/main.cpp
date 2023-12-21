#include <libstm-ota.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <driver/gpio.h>

using stm32::ota::InitConfig;

extern "C" void app_main() {
  InitConfig init_config = {
      .uarts = {InitConfig::Uart{
          .port = 1,
          .baud_rate = 9600,
          .tx = GPIO_NUM_17,
          .rx = GPIO_NUM_16,
          .parity = UART_PARITY_DISABLE,
      }},
      .spi_buses = {InitConfig::SPIBus{
          .port = 2,
          .miso = GPIO_NUM_19,
          .mosi = GPIO_NUM_23,
          .sclk = GPIO_NUM_18,
      }},
      .stm32bls = {InitConfig::STM32BL{
          .id = 1,
          .spi_port_id = 2,
          .cs = GPIO_NUM_5,
      }},
      .stm32s = {InitConfig::STM32{
          .id = 2,
          .reset = GPIO_NUM_22,
          .boot0 = GPIO_NUM_21,
          .bl_id = 1,
      }},
      .serial_proxies = {InitConfig::SerialProxy{.id = 1, .uart_port_id = 1}},
      .network_profiles = {InitConfig::NetworkProfile{
                               .id = 2,
                               .is_ap = true,
                               .is_static = false,
                               .ssid = "ESP32",
                               .password = "esp32-network",
                               .hostname = "esp32",
                               .ip = 0xc0a80001,
                               .subnet = 0xffffff00,
                               .gateway = 0xc0a80001,
                           },
                           InitConfig::NetworkProfile{
                               .id = 3,
                               .is_ap = false,
                               .is_static = false,
                               .ssid = "3-303-Abe 2.4Ghz",
                               .password = "syochnetwork",
                               .hostname = "esp32",
                               .ip = 0,
                               .subnet = 0,
                               .gateway = 0,
                           }},
      .active_network_profile_id = 3,
      .primary_stm32_id = 2};

  stm32::ota::OTAServer server(idf::GPIONum(22), init_config);

  while (1) vTaskDelay(1);
}