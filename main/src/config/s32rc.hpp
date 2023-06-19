#pragma once

#include "../libs/nvs_proxy.hpp"
#include <debugger_master.hpp>
namespace profile {
class STM32RemoteControllerProfile {
 private:
  nvs::SharedNamespace ns;

 public:
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<uint8_t> uart_tx;
  nvs::Proxy<uint8_t> uart_rx;

  STM32RemoteControllerProfile(nvs::SharedNamespace ns);
  void Save();
  DebuggerMaster* GetDebuggerMaster();
};
}  // namespace profile