#include "config.hpp"
#include <nvs.h>
#include "../init/init.hpp"

using namespace profile;

using gCfg = config::Config;

namespace profile {
using namespace types;
using stm32::driver::BLDriver;
std::shared_ptr<BLDriver> STM32BL::GetDriver() const {
  using enum types::BusType;
  switch ((types::BusType)this->bus_type) {
    case SPI:
      return BLDriver::SPIDriver(gCfg::GetSPIBus((uint8_t)this->bus_port),
                                 (idf::CS)this->cs.Get());
    case UART:
      return BLDriver::UARTDriver(gCfg::GetUARTPort((uint8_t)this->bus_port));
    default:
      throw std::runtime_error("Invalid bus type");
  }
}
}  // namespace profile

namespace config {

Server server;
wifi::Wifi network;
}  // namespace config