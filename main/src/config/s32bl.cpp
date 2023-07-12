#include "s32bl.hpp"

namespace profile {

STM32BLProfileInterface::~STM32BLProfileInterface() = default;

STM32BLProfileInterface* LoadSTM32BootLoaderProfile(nvs::SharedNamespace ns) {
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

UartSTM32BootLoaderProfile::~UartSTM32BootLoaderProfile() = default;

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

STM32BootLoaderUart* UartSTM32BootLoaderProfile::GetLoader() {
  static STM32BootLoaderUart* loader = nullptr;
  if (loader == nullptr) {
    loader = new STM32BootLoaderUart(
        (idf::GPIONum)(uint8_t)reset, (idf::GPIONum)(uint8_t)boot0,
        (uart_port_t)uart_port, (gpio_num_t)uart_tx, (gpio_num_t)uart_rx);
  }
  return loader;
}

SpiSTM32BootLoaderProfile::~SpiSTM32BootLoaderProfile() = default;

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

STM32BootLoaderSPI* SpiSTM32BootLoaderProfile::GetLoader() {
  static STM32BootLoaderSPI* loader = nullptr;
  if (loader == nullptr) {
    // TODO: FIX THIS
    // loader = new STM32BootLoaderSPI(
    //     (idf::GPIONum)(uint8_t)this->reset,
    //     (idf::GPIONum)(uint8_t)this->boot0, (idf::SPINum)spi_port,
    //     (idf::CS)(uint8_t)this->cs);
  }
  return loader;
}
}  // namespace profile