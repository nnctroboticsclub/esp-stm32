#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <driver/gpio.h>
#include "libs/simple_serial.hpp"
#include "libs/wifi.hpp"
#include "server.hpp"

// #define USE_NETWORK

// #define USE_DATA_SERVER

#if !defined(USE_NETWORK) && defined(USE_DATA_SERVER)
#error "USE_DATA_SERVER requires USE_NETWORK"
#endif

#define RESET GPIO_NUM_19
#define BOOT0 GPIO_NUM_21
#define led GPIO_NUM_2

namespace config {

extern simple_serial::Rx rx;
extern simple_serial::Tx tx;

#ifdef USE_NETWORK
extern app::Wifi network;
#endif

#ifdef USE_DATA_SERVER
extern Server server;
#endif
}  // namespace config

#endif