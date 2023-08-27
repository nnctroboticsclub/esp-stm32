#include "./config.hpp"
#include <nvs.h>
#include "../init/init.hpp"

using gCfg = config::Config;

namespace profile {

using stm32::driver::BLDriver;

std::shared_ptr<BLDriver> STM32BL::GetDriver() {
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

std::shared_ptr<stm32::STM32> STM32::Get() {
  if (!stm32) {
    auto bl = gCfg::GetSTM32BL(this->bl_id.Get());

    stm32 = std::make_shared<stm32::STM32>((idf::GPIONum)this->boot0.Get(),
                                           (idf::GPIONum)this->reset.Get(), bl);
  }
  return stm32;
}

}  // namespace profile

namespace config {

Config *Config::instance;

// Server server;
wifi::Wifi network;
}  // namespace config