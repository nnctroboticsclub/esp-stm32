#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

STM32BootLoader::STM32BootLoader(gpio_num_t reset, gpio_num_t boot0)
    : reset(reset), boot0(boot0) {
  gpio_set_direction(this->reset, GPIO_MODE_OUTPUT);
  gpio_set_direction(this->boot0, GPIO_MODE_OUTPUT);
  gpio_set_level(this->reset, 1);
  gpio_set_level(this->boot0, 0);
}

void STM32BootLoader::BootBootLoader() {
  ESP_LOGI(TAG, "Booting Bootloader");
  gpio_set_level(this->boot0, 1);
  vTaskDelay(20 / portTICK_PERIOD_MS);
  gpio_set_level(this->reset, 0);
  vTaskDelay(20 / portTICK_PERIOD_MS);
  gpio_set_level(this->reset, 1);
  vTaskDelay(20 / portTICK_PERIOD_MS);
  gpio_set_level(this->boot0, 0);
  vTaskDelay(50 / portTICK_PERIOD_MS);
}