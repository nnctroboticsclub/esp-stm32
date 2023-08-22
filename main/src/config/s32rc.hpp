#pragma once

#include <string>
#include <debugger_master.hpp>

#include "../libs/nvs_proxy.hpp"

namespace profile {
class STM32RemoteControllerProfile : public nvs::Namespace {
 public:
  [[no_unique_address]] nvs::Proxy<uint8_t> uart_port;
  [[no_unique_address]] nvs::Proxy<uint8_t> uart_tx;
  [[no_unique_address]] nvs::Proxy<uint8_t> uart_rx;

  explicit STM32RemoteControllerProfile(std::string const &ns);
  DebuggerMaster GetDebuggerMaster();
};
}  // namespace profile
