#include <stm32bl/helper.hpp>

#include <ranges>
#include <algorithm>

namespace connection::application::stm32bl {

static std::pair<uint16_t, uint16_t> ToFlashPageRange(uint32_t begin,
                                                      uint32_t end,
                                                      uint32_t flash_base) {
  auto begin_page = static_cast<uint16_t>((begin - flash_base) >> 11);
  auto end_page = static_cast<uint16_t>((end - flash_base) >> 11);
  return std::make_pair(begin_page, end_page);
}

Pages MemoryRangeToPages(uint32_t address, uint32_t length) {
  Pages pages;

  if (address < 0x08000000 && 0x0804'0000 < address + length) {
    pages.bank1 = true;

    auto [begin, end] =
        ToFlashPageRange(0x0804'0000, address + length, 0x0804'0000);

    for (uint16_t i = begin; i < end; i++) {
      pages.pages.push_back(i);
    }
  } else {
    auto [begin, end] =
        ToFlashPageRange(address, address + length, 0x0800'0000);

    for (uint16_t i = begin; i < end; i++) {
      pages.pages.push_back(i);
    }
  }
  return pages;
}

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

uint8_t CalculateChecksum(std::vector<uint8_t> buf) {
  uint8_t checksum{0b0000'0000};
  std::ranges::for_each(buf, [&checksum](uint8_t byte) { checksum ^= byte; });

  return checksum;
}
}  // namespace connection::application::stm32bl

namespace std {
std::string to_string(connection::application::stm32bl::SpecialFlashPage page) {
  return connection::application::stm32bl::SpecialFlashPageToString(page);
}
}  // namespace std