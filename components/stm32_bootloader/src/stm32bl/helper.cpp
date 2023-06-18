#include <stm32bl/helper.hpp>

namespace stm32bl {

Pages MemoryRangeToPages(uint32_t address, uint32_t length) {
  Pages pages;
  if (address < 0x08000000 && 0x0804'0000 < address + length) {
    // bulk erase bank1
    pages.bank1 = true;

    int bank2_erase_start = (address - 0x0804'0000) >> 11;
    int bank2_erase_end = (address + length - 0x0804'0000) >> 11;

    for (int i = bank2_erase_start; i < bank2_erase_end; i++) {
      pages.pages.push_back(i);
    }
  } else {
    int bank1_erase_start = (address - 0x0800'0000) >> 11;
    int bank1_erase_end = (address + length - 0x08'000000) >> 11;

    for (int i = bank1_erase_start; i < bank1_erase_end; i++) {
      pages.pages.push_back(i);
    }
  }
  return pages;
}

std::string SpecialFlashPageToString(SpecialFlashPage page) {
  switch (page) {
    case SpecialFlashPage::kGlobal:
      return "Global";
    case SpecialFlashPage::kBank1:
      return "Bank1";
    case SpecialFlashPage::kBank2:
      return "Bank2";
    default:
      return "Unknown";
  }
}

uint8_t CalculateChecksum(uint8_t* buf, size_t size) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < size; i++) {
    checksum ^= buf[i];
  }
  return checksum;
}
}  // namespace stm32bl