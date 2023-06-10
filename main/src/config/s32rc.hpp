#pragma once

#include "../libs/nvs_proxy.hpp"

namespace profile {
class STM32RemoteControllerProfile {
 public:
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<uint8_t> uart_tx;
  nvs::Proxy<uint8_t> uart_rx;

  STM32RemoteControllerProfile(nvs::Namespace* ns);
};
}  // namespace profile