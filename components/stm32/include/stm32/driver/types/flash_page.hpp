#pragma once

#include <cinttypes>
#include <vector>
#include <string>

namespace stm32::driver {
using FlashPage = uint16_t;
using FlashPages = std::vector<FlashPage>;
enum class SpecialFlashPage : uint16_t {
  kGlobal = 0xffff,
  kBank1 = 0xfffe,
  kBank2 = 0xfffd,
};

// TODO(syoch): Move this to somewhere else
std::string SpecialFlashPageToString(SpecialFlashPage page) {
  using enum SpecialFlashPage;
  switch (page) {
    case kGlobal:
      return "Global";
    case kBank1:
      return "Bank1";
    case kBank2:
      return "Bank2";
    default:
      return "Unknown";
  }
}

struct ErasePages {
  std::vector<FlashPages> normal_pages;
  std::vector<SpecialFlashPage> special_pages;
};
}  // namespace stm32::driver