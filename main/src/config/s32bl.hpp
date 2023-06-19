#pragma once

#include "../libs/nvs_proxy.hpp"
#include <stm32bl.hpp>
#include <stm32bl/stm32bl_spi.hpp>
#include <stm32bl/stm32bl_uart.hpp>
namespace profile {
class STM32BootLoaderProfileInterface {
 public:
  virtual void Save();
  virtual stm32bl::STM32BootLoader* GetLoader();
};

STM32BootLoaderProfileInterface* LoadSTM32BootLoaderProfile(nvs::Namespace* ns);

class UartSTM32BootLoaderProfile : public STM32BootLoaderProfileInterface {
 private:
  std::shared_ptr<nvs::Namespace> ns;

 public:
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<gpio_num_t> uart_tx;
  nvs::Proxy<gpio_num_t> uart_rx;

  UartSTM32BootLoaderProfile(nvs::Namespace* ns);
  void Save() override;
  stm32bl::Stm32BootLoaderUart* GetLoader() override;
};
class SpiSTM32BootLoaderProfile : public STM32BootLoaderProfileInterface {
 private:
  std::shared_ptr<nvs::Namespace> ns;

 public:
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<gpio_num_t> cs;
  nvs::Proxy<uint8_t> spi_port;

  SpiSTM32BootLoaderProfile(nvs::Namespace* ns);
  void Save() override;
  stm32bl::Stm32BootLoaderSPI* GetLoader() override;
};
}  // namespace profile