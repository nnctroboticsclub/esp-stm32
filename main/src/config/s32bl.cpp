#include "s32bl.hpp"

namespace profile {

STM32BootLoaderProfileInterface* LoadSTM32BootLoaderProfile(
    nvs::Namespace* ns) {
  nvs::Proxy<uint8_t> type(ns, "type");
  switch (type) {
    case 0:
      return new UartSTM32BootLoaderProfile(ns);
    case 1:
      return new SpiSTM32BootLoaderProfile(ns);
    default:
      ESP_LOGE("s32bl", "Invalid type %d", type);
      abort();
      return nullptr;
  }
}

UartSTM32BootLoaderProfile::UartSTM32BootLoaderProfile(nvs::Namespace* ns)
    : ns(ns),
      reset(ns, "reset"),
      boot0(ns, "boot0"),
      uart_port(ns, "uart_port"),
      uart_tx(ns, "uart_tx"),
      uart_rx(ns, "uart_rx") {}

void UartSTM32BootLoaderProfile::Save() { this->ns->Commit(); }

STMBootLoader* UartSTM32BootLoaderProfile::GetLoader() {
  static STMBootLoader* loader = nullptr;
  if (loader == nullptr) {
    loader = new STMBootLoader((gpio_num_t)reset, (gpio_num_t)boot0,
                               (uart_port_t)uart_port, (gpio_num_t)uart_tx,
                               (gpio_num_t)uart_rx);
  }
  return loader;
}

SpiSTM32BootLoaderProfile::SpiSTM32BootLoaderProfile(nvs::Namespace* ns)
    : ns(ns),
      reset(ns, "reset"),
      boot0(ns, "boot0"),
      cs(ns, "cs"),
      spi_port(ns, "spi_port") {}

void SpiSTM32BootLoaderProfile::Save() { this->ns->Commit(); }

stm32bl::Stm32BootLoaderSPI* SpiSTM32BootLoaderProfile::GetLoader() {
  static stm32bl::Stm32BootLoaderSPI* loader = nullptr;
  if (loader == nullptr) {
    SPIMaster bus{(spi_host_device_t)(uint8_t)this->spi_port};
    loader = new stm32bl::Stm32BootLoaderSPI((gpio_num_t)this->reset,
                                             (gpio_num_t)this->boot0, bus,
                                             (gpio_num_t)this->cs);
  }
  return loader;
}
}  // namespace profile