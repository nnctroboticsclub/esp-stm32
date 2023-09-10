#pragma once

#include <vector>
#include <memory>

#include "types/commands.hpp"
#include "types/version.hpp"
#include "types/flash_page.hpp"
#include "types/error.hpp"

#include "../raw_driver/raw_driver.hpp"

#include <stream/datalink/uart.hpp>

namespace stm32::session {
class BootLoaderSession;
}

namespace stm32::driver {
using raw_driver::RawDriverBase;

class BLDriver {
  static constexpr const char *TAG = "[STM32-BL] Driver";

  std::shared_ptr<RawDriverBase> raw_driver_;

  bool is_connected_ = false;

  Version version;
  Commands commands;

  void Get();
  void DoGetVersion();

  void DoErase(SpecialFlashPage page) const;
  void DoErase(const std::vector<FlashPage> &pages) const;

 public:
  explicit BLDriver(std::shared_ptr<RawDriverBase> raw_driver);
  void InitConnection();

  Version GetVersion();
  void Erase(SpecialFlashPage page) const;
  void Erase(std::vector<FlashPage> const &pages) const;
  void WriteMemoryBlock(uint32_t address,
                        const std::vector<uint8_t> &buffer) const;
  void Go(uint32_t address) const;

  static std::shared_ptr<BLDriver> SPIDriver(
      std::shared_ptr<idf::SPIMaster> master, idf::CS chip_select);

  static inline std::shared_ptr<BLDriver> UARTDriver(
      std::shared_ptr<stream::datalink::UART> uart) {
    auto raw_driver = std::make_shared<raw_driver::UartRawDriver>(uart);
    return std::make_shared<BLDriver>(raw_driver);
  }
};
}  // namespace stm32::driver
