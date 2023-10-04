#include "s32rc.hpp"

#include <stream/datalink/spi.hpp>
#include <stream/datalink/uart.hpp>

#include <data_proxy/handler.hpp>

#include "../config/config.hpp"
#include "./data_proxy_master.hpp"

namespace profile {
using esp_stm32::data_proxy::Handler;
using nvs::Namespace;

using stream::RecvAndSend;

std::shared_ptr<Handler> DataProxyProfile::Get() {
  using enum types::BusType;
  if (cache) return cache;

  using gCfg = config::Config;

  std::shared_ptr<RecvAndSend> io;
  if (bus_type.Get() == SPI) {
    auto bus = gCfg::GetSPIBus((uint8_t)this->bus_port);
    auto dev_ = std::make_shared<stream::datalink::SPIDevice>(
        bus, (idf::CS)this->cs.Get());
    io = std::dynamic_pointer_cast<RecvAndSend>(dev_);
  } else if (bus_type.Get() == UART) {
    auto port_ = gCfg::GetUARTPort((uint8_t)this->bus_port);
    io = std::dynamic_pointer_cast<RecvAndSend>(port_);
  } else {
    throw std::runtime_error("Unknown bus type");
  }

  esp_stm32::data_proxy::Link link{io};
  return std::make_shared<esp_stm32::data_proxy::ESPMaster>(link);
}
}  // namespace profile