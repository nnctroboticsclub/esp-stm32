#include "s32bl.hpp"

namespace profile {
STM32BootLoaderProfile::STM32BootLoaderProfile(nvs::Namespace* ns)
    : reset(ns, "reset"),
      boot0(ns, "boot0"),
      uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}
}  // namespace profile