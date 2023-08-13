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

 private:
  void AddPages(uint32_t begin, uint32_t end, uint32_t base) {
    auto begin_page = static_cast<uint16_t>((begin - base) >> 11);
    auto end_page = static_cast<uint16_t>((end - base) >> 11);

    for (uint16_t i = begin_page; i < end_page; i++) {
      this->normal_pages.emplace_back(i);
    }
  }

 public:
  ErasePages() = default;
  ErasePages(uint32_t begin, uint32_t length) {
    uint32_t end = begin + length;
    if (begin < 0x08000000 && 0x0804'0000 < end) {
      this->special_pages.push_back(SpecialFlashPage::kBank1);

      this->AddPages(0x0804'0000, end, 0x0804'0000);
    } else {
      this->AddPages(begin, end, 0x0800'0000);
    }
  }
};
}  // namespace stm32::driver