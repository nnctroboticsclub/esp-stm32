#pragma once

#include <vector>
#include <string>

namespace stm32bl {
using FlashPage = uint16_t;
enum SpecialFlashPage : uint16_t {
  kGlobal = 0xffff,
  kBank1 = 0xfffe,
  kBank2 = 0xfffd,
};

class Pages {
 public:
  bool bank1 = false;
  bool bank2 = false;
  std::vector<FlashPage> pages;
};

std::string SpecialFlashPageToString(SpecialFlashPage page);

Pages MemoryRangeToPages(uint32_t address, uint32_t length);

uint8_t CalculateChecksum(uint8_t* buf, size_t size);
}  // namespace stm32bl