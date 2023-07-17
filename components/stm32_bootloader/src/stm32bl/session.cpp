#include "stm32bl/session.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace connection::session {

STM32BL::STM32BL(idf::GPIONum reset, idf::GPIONum boot0)
    : reset(reset), boot0(boot0) {}

void STM32BL::TurnOnBoot1() { this->boot0.set_high(); }
void STM32BL::TurnOffBoot1() { this->boot0.set_low(); }

void STM32BL::Reset() {
  this->reset.set_low();
  vTaskDelay(50 / portTICK_PERIOD_MS);
  this->reset.set_high();
  vTaskDelay(50 / portTICK_PERIOD_MS);
}
}  // namespace connection::session