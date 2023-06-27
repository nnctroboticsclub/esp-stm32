#include "s32bl.hpp"

namespace profile {

STM32BootLoaderProfileInterface::~STM32BootLoaderProfileInterface() {}

STM32BootLoaderProfileInterface* LoadSTM32BootLoaderProfile(
    nvs::SharedNamespace ns) {
  nvs::Proxy<uint8_t> type(ns, "type");
  switch (type) {
    case 0:
      return new UartSTM32BootLoaderProfile(ns);
    case 1:
      return new SpiSTM32BootLoaderProfile(ns);
    default:
      ESP_LOGE("s32bl", "Invalid type %d", (uint8_t)type);
      abort();
      return nullptr;
  }
}

UartSTM32BootLoaderProfile::~UartSTM32BootLoaderProfile() {}

UartSTM32BootLoaderProfile::UartSTM32BootLoaderProfile(nvs::SharedNamespace ns)
    : ns(ns),
      reset(ns, "reset"),
      boot0(ns, "boot0"),
      uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}

void UartSTM32BootLoaderProfile::Save() {
  this->ns->Commit();
  nvs::Proxy<uint8_t> type(ns, "type");
  type = 0;
}

using stm32bl::Stm32BootLoaderUart;

Stm32BootLoaderUart* UartSTM32BootLoaderProfile::GetLoader() {
  static Stm32BootLoaderUart* loader = nullptr;
  if (loader == nullptr) {
    loader = new Stm32BootLoaderUart((gpio_num_t)reset, (gpio_num_t)boot0,
                                     (uart_port_t)uart_port,
                                     (gpio_num_t)uart_tx, (gpio_num_t)uart_rx);
  }
  return loader;
}

SpiSTM32BootLoaderProfile::~SpiSTM32BootLoaderProfile() {}

SpiSTM32BootLoaderProfile::SpiSTM32BootLoaderProfile(nvs::SharedNamespace ns)
    : ns(ns),
      reset(ns, "reset"),
      boot0(ns, "boot0"),
      cs(ns, "cs"),
      spi_port(ns, "spi_port") {}

void SpiSTM32BootLoaderProfile::Save() {
  this->ns->Commit();
  nvs::Proxy<uint8_t> type(ns, "type");
  type = 1;
}

stm32bl::Stm32BootLoaderSPI* SpiSTM32BootLoaderProfile::GetLoader() {
  static stm32bl::Stm32BootLoaderSPI* loader = nullptr;
  if (loader == nullptr) {
    loader = new stm32bl::Stm32BootLoaderSPI(
        (gpio_num_t)this->reset, (gpio_num_t)this->boot0, (idf::SPINum)spi_port,
        (idf::CS)(uint8_t)this->cs);
  }
  return loader;
}
}  // namespace profile