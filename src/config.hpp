#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <driver/gpio.h>

#define led (gpio_num_t)2

#define tx_data (gpio_num_t) led
#define tx_clock (gpio_num_t)5
#define tx_check (gpio_num_t)4

#define rx_data (gpio_num_t)26
#define rx_clock (gpio_num_t)25
#define rx_check (gpio_num_t)27

#endif