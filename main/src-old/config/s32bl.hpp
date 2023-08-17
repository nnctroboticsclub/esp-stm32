#pragma once

#include "../libs/nvs_proxy.hpp"
#include <stm32bl.hpp>
#include <stm32bl/stm32bl_spi.hpp>
#include <stm32bl/stm32bl_uart.hpp>
namespace profile {

using connection::application::STM32BootLoader;
using connection::application::STM32BootLoaderSPI;
using connection::application::STM32BootLoaderUart;
class STM32BLProfileInterface {
 public:
  virtual ~STM32BLProfileInterface() = 0;
  virtual void Save() = 0;
  virtual STM32BootLoader* GetLoader() = 0;
};

STM32BLProfileInterface* LoadSTM32BootLoaderProfile(nvs::SharedNamespace ns);

class UartSTM32BootLoaderProfile : public STM32BLProfileInterface {
 public:
  nvs::SharedNamespace ns;

  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<gpio_num_t> uart_tx;
  nvs::Proxy<gpio_num_t> uart_rx;

  explicit UartSTM32BootLoaderProfile(nvs::SharedNamespace ns);
  ~UartSTM32BootLoaderProfile() override;

  void Save() override;
  STM32BootLoaderUart* GetLoader() override;
};
class SpiSTM32BootLoaderProfile : public STM32BLProfileInterface {
 public:
  nvs::SharedNamespace ns;

  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<gpio_num_t> cs;
  nvs::Proxy<uint8_t> spi_port;

  explicit SpiSTM32BootLoaderProfile(nvs::SharedNamespace ns);
  ~SpiSTM32BootLoaderProfile() override;

  void Save() override;
  STM32BootLoaderSPI* GetLoader() override;
};
}  // namespace profile