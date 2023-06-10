#pragma once

#include "../libs/nvs_proxy.hpp"

namespace profile {
class STM32BootLoaderProfile {
 public:
  nvs::Proxy<uint8_t> reset;
  nvs::Proxy<uint8_t> boot0;
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<uint8_t> uart_tx;
  nvs::Proxy<uint8_t> uart_rx;

  STM32BootLoaderProfile(nvs::Namespace* ns);
};
}  // namespace profile