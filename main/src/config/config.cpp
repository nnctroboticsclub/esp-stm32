#include "config.hpp"
#include "init/init.hpp"
#include <nvs.h>

namespace config {

Config::Config()
    : network_profiles{
          NetworkProfile(new nvs::Namespace("a_nw0")),
          NetworkProfile(new nvs::Namespace("a_nw1")),
          NetworkProfile(new nvs::Namespace("a_nw2")),
          NetworkProfile(new nvs::Namespace("a_nw3")),
          NetworkProfile(new nvs::Namespace("a_nw4")),
      },
      active_network_profile(new nvs::Namespace("a_conf"), "active_nw"),
      server_profile(new nvs::Namespace("a_srv")),
      stm32_bootloader_profile(new nvs::Namespace("a_s32bl")),
      stm32_remote_controller_profile(new nvs::Namespace("a_s32rc")) {}

#ifdef CONFIG_STM32_BOOTLOADER_DRIVER
STMBootLoader loader((gpio_num_t)CONFIG_STM32_BOOTLOADER_RESET,
                     (gpio_num_t)CONFIG_STM32_BOOTLOADER_BOOT0,
                     CONFIG_STM32_BOOTLOADER_UART,
                     CONFIG_STM32_BOOTLOADER_UART_TX,
                     CONFIG_STM32_BOOTLOADER_UART_RX);
#endif

#ifdef CONFIG_STM32_REMOTE_CONTROLLER_DRIVER
DebuggerMaster debugger(CONFIG_STM32_REMOTE_CONTROLLER_UART,
                        CONFIG_STM32_REMOTE_CONTROLLER_TX,
                        CONFIG_STM32_REMOTE_CONTROLLER_RX);
#endif

app::Wifi network;

Server server;
}  // namespace config