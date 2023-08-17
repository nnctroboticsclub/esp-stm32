#include <stm32/driver/types/commands.hpp>

namespace stm32::driver {
Commands::Commands() = default;
Commands::Commands(const std::vector<uint8_t> &data) {
  if (data.size() < 0x0b) {
    ESP_LOGE("Stm32BL", "data size is too small: %d bytes", data.size());

    throw InvalidData();
  }
  this->get = data[0];
  this->get_version = data[1];
  this->get_id = data[2];
  this->read_memory = data[3];
  this->go = data[4];
  this->write_memory = data[5];
  this->erase = data[6];
  this->write_protect = data[7];
  this->write_unprotect = data[8];
  this->readout_protect = data[9];
  this->readout_unprotect = data[10];
  if (data.size() > 0x0c) {
    this->get_checksum = data[11];
  }
}
}  // namespace stm32::driver
