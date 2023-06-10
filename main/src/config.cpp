#include "config.hpp"
#include "init/init.hpp"
#include <nvs.h>

namespace config {

NetworkProfile::NetworkProfile(nvs::Namespace* ns)
    : mode(ns, "mode"),
      name(ns, "name"),
      ssid(ns, "ssid"),
      password(ns, "password"),
      hostname(ns, "hostname"),
      ip(ns, "ip"),
      subnet(ns, "subnet"),
      gateway(ns, "gateway"),
      dns(ns, "dns") {}

ServerProfile::ServerProfile(nvs::Namespace* ns)
    : ip(ns, "ip"), port(ns, "port") {}

STM32BootLoaderProfile::STM32BootLoaderProfile(nvs::Namespace* ns)
    : reset(ns, "reset"),
      boot0(ns, "boot0"),
      uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}

STM32RemoteControllerProfile::STM32RemoteControllerProfile(nvs::Namespace* ns)
    : uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}

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
      stm32_bootloader_profile(new nvs::Namespace("a_stm32_bl")),
      stm32_remote_controller_profile(new nvs::Namespace("a_stm32_rc")) {}

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

#ifdef CONFIG_USE_NETWORK
app::Wifi network(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
#endif

#ifdef CONFIG_USE_DATA_SERVER
Server server;
#endif
}  // namespace config