#include "gpio.hpp"
namespace app {

DigitalPinWrapper::DigitalPinWrapper(gpio_num_t pin, gpio_mode_t mode)
    : pin(pin) {
  gpio_set_direction(pin, mode);
}

void DigitalPinWrapper::Set(bool x) { gpio_set_level(pin, x); }
bool DigitalPinWrapper::Get() { return gpio_get_level(pin); }

DigitalPinWrapper::operator bool() { return this->Get(); }

void DigitalPinWrapper::operator=(DigitalPinWrapper &y) { this->Set(y.Get()); }

DigitalOut::DigitalOut(gpio_num_t pin)
    : DigitalPinWrapper(pin, GPIO_MODE_OUTPUT) {}

}  // namespace app

bool operator<<(app::DigitalPinWrapper &pin, bool x) {
  pin.Set(static_cast<bool>(x));
  return x;
}

int operator<<(app::DigitalPinWrapper &pin, int x) {
  pin.Set(x != 0);
  return x;
}