#include "s32rc.hpp"

#include <connection/data_link/spi.hpp>
#include <connection/data_link/uart.hpp>

#include "./config.hpp"

namespace profile {
using data_proxy::Proxy;
using nvs::Namespace;

using connection::data_link::RecvAndSend;

std::shared_ptr<Proxy> DataProxyProfile::Get() {
  using enum types::BusType;
  if (cache) return cache;

  using gCfg = config::Config;

  RecvAndSend* io_;
  if (bus_type.Get() == SPI) {
    auto bus = gCfg::GetSPIBus((uint8_t)this->bus_port);
    auto dev_ = connection::data_link::SPIDevice(bus, (idf::CS)this->cs.Get());
    io_ = dynamic_cast<RecvAndSend*>(&dev_);
  } else if (bus_type.Get() == UART) {
    auto port_ = gCfg::GetUARTPort((uint8_t)this->bus_port).get();
    io_ = dynamic_cast<RecvAndSend*>(port_);
  } else {
    throw std::runtime_error("Unknown bus type");
  }

  auto io = std::shared_ptr<RecvAndSend>(io_);
  data_proxy::Link link{io};
  return std::make_shared<Proxy>(link);
}
}  // namespace profile