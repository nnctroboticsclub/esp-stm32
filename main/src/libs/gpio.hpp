#pragma once
#ifndef GPIO_HPP
#define GPIO_HPP

#include <driver/gpio.h>

namespace app {

class DigitalPinWrapper {
  gpio_num_t pin;

 public:
  DigitalPinWrapper(gpio_num_t pin, gpio_mode_t mode = GPIO_MODE_OUTPUT);

  void Set(bool x);
  bool Get();

  operator bool();
  void operator=(DigitalPinWrapper &y);
};

class DigitalOut : public DigitalPinWrapper {
 public:
  DigitalOut(gpio_num_t pin);
};

}  // namespace app

bool operator<<(app::DigitalPinWrapper &pin, bool x);
int operator<<(app::DigitalPinWrapper &pin, int x);

#endif