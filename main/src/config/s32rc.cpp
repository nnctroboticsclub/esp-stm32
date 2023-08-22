#include "./s32rc.hpp"

namespace profile {
using nvs::Namespace;
STM32RemoteControllerProfile::STM32RemoteControllerProfile(
    std::string const& ns)
    : Namespace(ns),
      uart_port(this, "uart_port"),
      uart_tx(this, "uart_tx"),
      uart_rx(this, "uart_rx") {}

DebuggerMaster STM32RemoteControllerProfile::GetDebuggerMaster() {
  return DebuggerMaster(this->uart_port.Get(), this->uart_tx.Get(),
                        this->uart_rx.Get());
}
}  // namespace profile