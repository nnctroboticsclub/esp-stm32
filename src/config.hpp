#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <driver/gpio.h>
#include "libs/simple_serial.hpp"
#include "libs/wifi.hpp"

#define led (gpio_num_t)2

namespace config {
extern app::Wifi network;

extern simple_serial::Rx rx;
extern simple_serial::Tx tx;
}  // namespace config

#endif