#include <stm32.hpp>
#include <bin.h>

extern "C" int app_main() {
  idf::SPIMaster master{(idf::SPINum)2, (idf::MOSI)23, (idf::MISO)19,
                        (idf::SCLK)18};

  connection::data_link::SPIDevice device{master, (idf::CS)5};

  stm32::raw_driver::SpiRawDriver raw_driver{
      std::make_shared<connection::data_link::SPIDevice>(device)};

  stm32::session::Session session{
      std::make_shared<stm32::raw_driver::SpiRawDriver>(raw_driver),
      (idf::GPIONum)21, (idf::GPIONum)22};

  auto _bl = session.TryEnterBL(2);
  if (!_bl) {
    ESP_LOGE("main", "Failed to enter bootloader");
    return 1;
  }
  auto bl = *_bl;

  bl.Erase(stm32::driver::ErasePages(0x08000000, new_flash_len));

  std::vector<uint8_t> flash(new_flash, new_flash + new_flash_len);

  bl.WriteMemory(0x08000000, flash);

  return 0;
}