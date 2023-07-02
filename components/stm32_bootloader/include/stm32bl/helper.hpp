#pragma once

#include <vector>
#include <string>
#include <stdexcept>

namespace connection::application::stm32bl {
using FlashPage = uint16_t;
enum class SpecialFlashPage : uint16_t {
  kGlobal = 0xffff,
  kBank1 = 0xfffe,
  kBank2 = 0xfffd,
};

class NotImplemented : public std::runtime_error {
 public:
  NotImplemented() : std::runtime_error("Not Implemented") {}
};

class ACKFailed : public std::runtime_error {
 public:
  ACKFailed() : std::runtime_error("ACK Failed") {}
};

class BootLoaderNotBooted : public std::runtime_error {
 public:
  BootLoaderNotBooted() : std::runtime_error("BootLoader not booted") {}
};

enum class ACK : uint8_t {
  ACK = 0x79,
  NACK = 0x1f,
};

class Pages {
 public:
  bool bank1 = false;
  bool bank2 = false;
  std::vector<FlashPage> pages;
};

std::string SpecialFlashPageToString(SpecialFlashPage page);

Pages MemoryRangeToPages(uint32_t address, uint32_t length);

uint8_t CalculateChecksum(std::vector<uint8_t> buf);

}  // namespace connection::application::stm32bl