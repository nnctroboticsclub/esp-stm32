#pragma once

#include <vector>

#include <freertos/FreeRTOS.h>

#include "stm32bl/helper.hpp"
#include "stm32bl/session.hpp"
#include <gpio_cxx.hpp>

namespace connection::application {
namespace stm32bl {
struct Commands {
  uint8_t get = 0x00;
  uint8_t get_version = 0x01;
  uint8_t get_id = 0x02;
  uint8_t read_memory = 0x11;
  uint8_t go = 0x21;
  uint8_t write_memory = 0x31;
  uint8_t erase = 0x43;
  uint8_t special = 0x50;
  uint8_t extended_special = 0x51;
  uint8_t write_protect = 0x63;
  uint8_t write_unprotect = 0x73;
  uint8_t readout_protect = 0x82;
  uint8_t readout_unprotect = 0x92;
  uint8_t get_checksum = 0xA1;

  Commands() = default;
  explicit Commands(std::vector<uint8_t> &data);

  inline bool UseLegacyErase() const { return this->erase == 0x44; }
};

struct Version {
  bool updated = false;
  uint8_t major = 0xee;
  uint8_t minor = 0xee;
  uint8_t option1 = 0xee;
  uint8_t option2 = 0xee;

  Version() = default;
  explicit Version(std::vector<uint8_t> &data);

  void UpdateVersion(uint8_t byte);
};

class OutboundData {
 public:
  enum class SizeMode { kNone = 0, kU16, kU8 };

  //* Datas
  std::vector<uint8_t> data;

  SizeMode size;
  bool with_checksum = false;
};

class STM32BootLoader {
  static constexpr const char *TAG = "STM32 BootLoader";

  session::STM32BL session;

  //* Some commands for transfering data

  virtual void CommandHeader(uint8_t cmd) = 0;
  virtual void SendAddress(uint32_t address) = 0;
  virtual void SendFlashPage(SpecialFlashPage address) = 0;
  virtual void SendFlashPage(std::vector<FlashPage> &address) = 0;
  virtual void SendDataWithChecksum(std::vector<uint8_t> &data) = 0;
  virtual void ReadData(std::vector<uint8_t> &buffer) = 0;
  virtual void ReadDataWithoutHeader(std::vector<uint8_t> &buffer) = 0;

  // * New!
  virtual void SendData(OutboundData &data) = 0;

  virtual void RecvACK(TickType_t timeout = 100 / portTICK_PERIOD_MS) = 0;

  //* some internal commands
  Commands commands;
  Version version;

  void WriteMemoryBlock(uint32_t address, std::vector<uint8_t> &buf);

  virtual void Erase(SpecialFlashPage page);
  virtual void Erase(std::vector<FlashPage> &pages);

  //* internal
  virtual void Sync() = 0;

 public:
  STM32BootLoader(idf::GPIONum reset, idf::GPIONum boot0);
  virtual ~STM32BootLoader();

  void Connect();

  //* Commands
  void Get();
  void GetVersion();
  void Go(uint32_t address);

  //* Utility functions
  void Erase(uint32_t address, uint32_t length);
  void Erase(Pages pages);
  void WriteMemory(uint32_t address, std::vector<uint8_t> &buf);
};
}  // namespace stm32bl

using STM32BootLoader = stm32bl::STM32BootLoader;
}  // namespace connection::application