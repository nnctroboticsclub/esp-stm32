#include "s32bl.hpp"

namespace profile {
STM32BootLoaderProfile::STM32BootLoaderProfile(nvs::Namespace* ns)
    : ns(ns),
      reset(ns, "reset"),
      boot0(ns, "boot0"),
      uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}

void STM32BootLoaderProfile::Save() { this->ns->Commit(); }

STMBootLoader* STM32BootLoaderProfile::GetLoader() {
  static STMBootLoader* loader = nullptr;
  if (loader == nullptr) {
    loader = new STMBootLoader((gpio_num_t)reset, (gpio_num_t)boot0,
                               (uart_port_t)uart_port, (gpio_num_t)uart_tx,
                               (gpio_num_t)uart_rx);
  }
  return loader;
}
}  // namespace profile