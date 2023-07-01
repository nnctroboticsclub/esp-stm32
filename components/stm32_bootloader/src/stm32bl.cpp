#include <stm32bl.hpp>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace stm32bl {
STM32BootLoader::STM32BootLoader(idf::GPIONum reset, idf::GPIONum boot0)
    : reset(reset), boot0(boot0) {
  this->reset.set_high();
  this->boot0.set_low();
}

STM32BootLoader::~STM32BootLoader() = default;

void STM32BootLoader::BootBootLoader() {
  ESP_LOGI(TAG, "Booting Bootloader");
  this->boot0.set_high();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  this->reset.set_low();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  this->reset.set_high();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  this->boot0.set_low();
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