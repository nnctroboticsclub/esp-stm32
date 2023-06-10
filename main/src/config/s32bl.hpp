#pragma once

#include "../libs/nvs_proxy.hpp"
#include <stmbootloader.hpp>
namespace profile {
class STM32BootLoaderProfile {
 private:
  std::shared_ptr<nvs::Namespace> ns;

 public:
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<gpio_num_t> uart_tx;
  nvs::Proxy<gpio_num_t> uart_rx;

  STM32BootLoaderProfile(nvs::Namespace* ns);
  void Save();
  STMBootLoader* GetLoader();
};
}  // namespace profile