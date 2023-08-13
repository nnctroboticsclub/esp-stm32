#pragma once

#include <esp_log.h>

#include <cinttypes>
#include <vector>

#include "./error.hpp"

namespace stm32::driver {
struct Commands {
  uint8_t get = 0x00;
  uint8_t get_version = 0x01;
  uint8_t get_id = 0x02;
  uint8_t read_memory = 0x11;
  uint8_t go = 0x21;
  uint8_t write_memory = 0x31;
  uint8_t erase = 0x43;
  uint8_t special = 0x50;
  uint8_t extended_special = 0x51;
  uint8_t write_protect = 0x63;
  uint8_t write_unprotect = 0x73;
  uint8_t readout_protect = 0x82;
  uint8_t readout_unprotect = 0x92;
  uint8_t get_checksum = 0xA1;

  Commands() = default;
  explicit Commands(const std::vector<uint8_t> &data) {
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

  inline bool UseLegacyErase() const { return this->erase == 0x44; }
};

}  // namespace stm32::driver
