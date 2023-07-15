#pragma once

#include <driver/gpio.h>
#include "../stm32bl.hpp"
#include "helper.hpp"
#include <connection/data_link/spi.hpp>

namespace connection::application {
namespace stm32bl {

class Stm32BootLoaderSPI : public stm32bl::STM32BootLoader {
  static constexpr const char *TAG = "STM32 BootLoader[SPI]";

  Commands commands;
  Version version;
  connection::data_link::SPIDevice device;
  uint8_t v;

  void RecvACK(TickType_t timeout = 100 / portTICK_PERIOD_MS) override;

  void Synchronization();

  void CommandHeader(uint8_t cmd) override;

  void SendAddress(uint32_t address) override;

  void SendDataWithChecksum(std::vector<uint8_t> &data) override;

  void ReadData(std::vector<uint8_t> &buf) override;

  void ReadDataWithoutHeader(std::vector<uint8_t> &buf) override;

  void Erase(SpecialFlashPage page) override;
  void Erase(std::vector<FlashPage> &pages) override;

 public:
  Stm32BootLoaderSPI(idf::GPIONum reset, idf::GPIONum boot0,
                     idf::SPIMaster &spi_host, idf::CS cs);
  ~Stm32BootLoaderSPI() override;

  void Connect() override;

  inline void SetDebugVarU8(uint8_t v) { this->v = v; }
};
}  // namespace stm32bl
using STM32BootLoaderSPI = stm32bl::Stm32BootLoaderSPI;

}  // namespace connection::application