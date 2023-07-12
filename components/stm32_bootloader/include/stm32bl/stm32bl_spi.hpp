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
  connection::data_link::SPIDevice device;
  uint8_t v;

  void WaitACKFrame();

  void Synchronization();

  void CommandHeader(uint8_t cmd);

  void ReadData(std::vector<uint8_t> &buf);

  void ReadDataWithoutHeader(std::vector<uint8_t> &buf);

 public:
  Stm32BootLoaderSPI(idf::GPIONum reset, idf::GPIONum boot0,
                     idf::SPIMaster &spi_host, idf::CS cs);
  ~Stm32BootLoaderSPI() override;

  void Connect() override;

  void Get();

  inline void SetDebugVarU8(uint8_t v) { this->v = v; }

  void Erase(SpecialFlashPage page) override;
  void Erase(std::vector<FlashPage> &pages) override;
  using STM32BootLoader::Erase;

  void WriteMemoryBlock(uint32_t addr, std::vector<uint8_t> &buffer) override;

  void Go(uint32_t addr) override;
};
}  // namespace stm32bl
using STM32BootLoaderSPI = stm32bl::Stm32BootLoaderSPI;

}  // namespace connection::application