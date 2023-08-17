#include "s32rc.hpp"

namespace profile {
STM32RemoteControllerProfile::STM32RemoteControllerProfile(nvs::SharedNamespace ns)
    : ns(ns),
      uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}

void STM32RemoteControllerProfile::Save() { this->ns->Commit(); }

DebuggerMaster* STM32RemoteControllerProfile::GetDebuggerMaster() {
  return new DebuggerMaster(this->uart_port, this->uart_tx, this->uart_rx);
}
}  // namespace profile