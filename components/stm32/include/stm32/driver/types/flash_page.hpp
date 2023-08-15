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

std::string SpecialFlashPageToString(SpecialFlashPage page);

struct ErasePages {
  FlashPages normal_pages;
  std::vector<SpecialFlashPage> special_pages;

 private:
  void AddPages(uint32_t begin, uint32_t end, uint32_t base);

 public:
  ErasePages();
  ErasePages(uint32_t begin, uint32_t length);
};
}  // namespace stm32::driver