#include "config.hpp"

namespace config {

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