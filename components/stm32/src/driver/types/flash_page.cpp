#include <stm32/driver/types/flash_page.hpp>

namespace stm32::driver {
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

void ErasePages::AddPages(uint32_t begin, uint32_t end, uint32_t base) {
  auto begin_page = static_cast<uint16_t>((begin - base) >> 11);
  auto end_page = static_cast<uint16_t>((end - base) >> 11);

  for (uint16_t i = begin_page; i < end_page; i++) {
    this->normal_pages.emplace_back(i);
  }
}
ErasePages::ErasePages() = default;
ErasePages::ErasePages(uint32_t begin, uint32_t length) {
  uint32_t end = begin + length;
  if (begin < 0x08000000 && 0x0804'0000 < end) {
    this->special_pages.push_back(SpecialFlashPage::kBank1);

    this->AddPages(0x0804'0000, end, 0x0804'0000);
  } else {
    this->AddPages(begin, end, 0x0800'0000);
  }
}
}  // namespace stm32::driver