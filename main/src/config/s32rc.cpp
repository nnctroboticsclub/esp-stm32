#include "s32rc.hpp"

namespace profile {
STM32RemoteControllerProfile::STM32RemoteControllerProfile(nvs::Namespace* ns)
    : uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}
}  // namespace profile