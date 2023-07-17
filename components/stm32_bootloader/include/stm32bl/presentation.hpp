#pragma once

#include <vector>

namespace connection::presentation::stm32bl {

std::vector<uint8_t> ToU8Vector(uint32_t value) {
  std::vector<uint8_t> buf(4);
  buf[0] = (value >> 24) & 0xff;
  buf[1] = (value >> 16) & 0xff;
  buf[2] = (value >> 8) & 0xff;
  buf[3] = value & 0xff;
  return buf;
}

std::vector<uint8_t> ToU8Vector(uint16_t value) {
  std::vector<uint8_t> buf(2);
  buf[0] = (value >> 8) & 0xff;
  buf[1] = value & 0xff;
  return buf;
}

std::vector<uint8_t> ToU8Vector(uint8_t value) {
  std::vector<uint8_t> buf(1);
  buf[0] = value;
  return buf;
}

}