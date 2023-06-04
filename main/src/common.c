#include "common.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void FatalError(const char* message) {
  ESP_LOGE("System", "Fatal Error: %s", message);
  while (1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}