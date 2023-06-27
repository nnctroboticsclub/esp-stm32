#pragma once

#include <driver/gpio.h>
#include "../stm32bl.hpp"
#include "helper.hpp"
#include <spi_host_cxx.hpp>

namespace stm32bl {

class Stm32BootLoaderSPI : public STM32BootLoader {
  static constexpr const char* TAG = "STM32 BootLoader[SPI]";

 public:  // Debug public.
  idf::SPIDevice device;

 private:
  struct {
    uint8_t get;
    uint8_t get_version;
    uint8_t get_id;
    uint8_t read_memory;
    uint8_t go;
    uint8_t write_memory;
    uint8_t erase;
    uint8_t special;
    uint8_t extended_special;
    uint8_t write_protect;
    uint8_t write_unprotect;
    uint8_t readout_protect;
    uint8_t readout_unprotect;
    uint8_t get_checksum;
  } commands;

  TaskResult WaitACKFrame();

  TaskResult Synchronization();

  TaskResult CommandHeader(uint8_t cmd);

  TaskResult ReadData(uint8_t* buf, size_t size);

  TaskResult ReadDataWithoutHeader(uint8_t* buf, size_t size);

 public:
  Stm32BootLoaderSPI(idf::GPIONum reset, idf::GPIONum boot0,
                     idf::SPINum spi_host, idf::CS cs);
  virtual ~Stm32BootLoaderSPI();

  TaskResult Connect();

  TaskResult Get();

  TaskResult Erase(SpecialFlashPage page);
  TaskResult Erase(std::vector<FlashPage> pages);
  TaskResult Erase(uint32_t addr, uint32_t size) override;

  TaskResult WriteMemoryBlock(uint32_t addr, std::vector<uint8_t> buffer);
  TaskResult WriteMemory(uint32_t addr, std::vector<uint8_t> buffer) override;

  TaskResult Go(uint32_t addr) override;
};
}  // namespace stm32bl