#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "libs/stmbootloader.hpp"
#include "libs/wifi.hpp"
#include "server.hpp"
#include "libs/debugger_master.hpp"

namespace config {

#ifdef CONFIG_STM32_BOOTLOADER_DRIVER
extern STMBootLoader loader;
#endif

#ifdef CONFIG_STM32_REMOTE_CONTROLLER_DRIVER
extern DebuggerMaster debugger;
#endif

#ifdef CONFIG_USE_NETWORK
extern app::Wifi network;
#endif

#ifdef CONFIG_USE_DATA_SERVER
extern Server server;
#endif
}  // namespace config

#endif