#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <driver/gpio.h>
#include "libs/stmbootloader.hpp"
#include "libs/wifi.hpp"
#include "server.hpp"

//* constant
#define USER_PROGRAM_START 0x08000000

//* features
// #define USE_NETWORK
#define USE_DATA_SERVER

//! global variables
#if !defined(USE_NETWORK) && defined(USE_DATA_SERVER)
#define USE_NETWORK
#endif

namespace config {

extern STMBootLoader loader;

#ifdef USE_NETWORK
extern app::Wifi network;
#endif

#ifdef USE_DATA_SERVER
extern Server server;
#endif
}  // namespace config

#endif