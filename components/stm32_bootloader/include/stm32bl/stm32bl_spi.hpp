#include <spi.hpp>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "helper.hpp"
namespace stm32bl {

class Stm32BootLoaderSPI {
  static constexpr const char* TAG = "STM32 BootLoader[SPI]";

 public:  // Debug public.
  SPIDevice device;

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

  gpio_num_t reset, boot0;
  void DoSTM32Reset();

  TaskResult WaitACKFrame();

  TaskResult Synchronization();

  TaskResult CommandHeader(uint8_t cmd);

  TaskResult ReadData(uint8_t* buf, size_t size);

  TaskResult ReadDataWithoutHeader(uint8_t* buf, size_t size);

 public:
  Stm32BootLoaderSPI(gpio_num_t reset, gpio_num_t boot0, SPIMaster& spi_master,
              int cs);

  TaskResult Connect();

  TaskResult Get();

  TaskResult Erase(SpecialFlashPage page);
  TaskResult Erase(std::vector<FlashPage> pages);
  TaskResult Erase(uint32_t addr, uint32_t size);

  TaskResult WriteMemoryBlock(uint32_t addr, uint8_t* buffer, size_t size);
  TaskResult WriteMemory(uint32_t addr, uint8_t* buffer, size_t size);
};
}  // namespace stm32bl