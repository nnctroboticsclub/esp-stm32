#include <libsta-ota.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

extern "C" void app_main() {
  stm32::ota::OTAServer server(idf::GPIONum(22));

  while (1) vTaskDelay(1);
}