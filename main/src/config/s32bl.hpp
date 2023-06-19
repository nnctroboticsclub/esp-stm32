#pragma once

#include "../libs/nvs_proxy.hpp"
#include <stm32bl.hpp>
#include <stm32bl/stm32bl_spi.hpp>
#include <stm32bl/stm32bl_uart.hpp>
namespace profile {
class STM32BootLoaderProfileInterface {
 public:
  virtual ~STM32BootLoaderProfileInterface();
  virtual void Save();
  virtual stm32bl::STM32BootLoader* GetLoader();
};

STM32BootLoaderProfileInterface* LoadSTM32BootLoaderProfile(
    nvs::SharedNamespace ns);

class UartSTM32BootLoaderProfile : public STM32BootLoaderProfileInterface {
 private:
  nvs::SharedNamespace ns;

 public:
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<gpio_num_t> uart_tx;
  nvs::Proxy<gpio_num_t> uart_rx;

  virtual ~UartSTM32BootLoaderProfile();

  UartSTM32BootLoaderProfile(nvs::SharedNamespace ns);
  void Save() override;
  stm32bl::Stm32BootLoaderUart* GetLoader() override;
};
class SpiSTM32BootLoaderProfile : public STM32BootLoaderProfileInterface {
 private:
  nvs::SharedNamespace ns;

 public:
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<gpio_num_t> cs;
  nvs::Proxy<uint8_t> spi_port;

  virtual ~SpiSTM32BootLoaderProfile();

  SpiSTM32BootLoaderProfile(nvs::SharedNamespace ns);
  void Save() override;
  stm32bl::Stm32BootLoaderSPI* GetLoader() override;
};
}  // namespace profile