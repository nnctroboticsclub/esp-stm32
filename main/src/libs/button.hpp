#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <gpio_cxx.hpp>

class UserButton {
  static constexpr const char* TAG = "UserButton";

 private:
  idf::GPIOInput pin;

 public:
  UserButton(idf::GPIONum button) : pin(button) {}

  bool IsPressed() { return this->pin.get_level() == idf::GPIOLevel::HIGH; }

  void WaitUntilPressed() {
    while (!this->IsPressed()) vTaskDelay(50 / portTICK_PERIOD_MS);

    while (this->IsPressed()) vTaskDelay(50 / portTICK_PERIOD_MS);
  }
};