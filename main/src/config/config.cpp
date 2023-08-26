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

std::shared_ptr<stm32::STM32> STM32::Get() {
  if (!stm32) {
    auto bl = gCfg::GetSTM32BL(this->bl_id.Get());
    ESP_LOGI("STM32 Profile", "Creating STM32 with BL ID %d[%p]",
             this->bl_id.Get(), bl.get());
    stm32 = std::make_shared<stm32::STM32>((idf::GPIONum)this->reset.Get(),
                                           (idf::GPIONum)this->boot0.Get(), bl);
  }
  return stm32;
}

}  // namespace profile

namespace config {

Config Config::instance;

// Server server;
wifi::Wifi network;
}  // namespace config