#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace stm32bl {
STM32BootLoader::STM32BootLoader(gpio_num_t reset, gpio_num_t boot0)
    : reset(reset), boot0(boot0) {
  gpio_set_direction(this->reset, GPIO_MODE_OUTPUT);
  gpio_set_direction(this->boot0, GPIO_MODE_OUTPUT);
  gpio_set_level(this->reset, 1);
  gpio_set_level(this->boot0, 0);
}

STM32BootLoader::~STM32BootLoader() {}

void STM32BootLoader::BootBootLoader() {
  ESP_LOGI(TAG, "Booting Bootloader (boot0: %d, reset: %d)", this->boot0,
           this->reset);
  gpio_set_level(this->boot0, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(this->reset, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(this->reset, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(this->boot0, 0);
  vTaskDelay(50 / portTICK_PERIOD_MS);
}

TaskResult STM32BootLoader::Erase(uint32_t address, uint32_t length,
                                  uint32_t size) {
  uint32_t pointer = address;
  uint32_t end = address + length;
  while (pointer + size < end) {
    RUN_TASK_V(this->Erase(pointer, size));
    pointer += size;
  }

  if (pointer < end) {
    RUN_TASK_V(this->Erase(pointer, end - pointer));
  }

  return TaskResult::Ok();
}

}  // namespace stm32bl