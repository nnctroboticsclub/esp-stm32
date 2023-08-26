#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <vector>
#include <cinttypes>
#include <type_traits>

#include "../types/inbounddata.hpp"
#include "../types/outbounddata.hpp"

namespace stm32 {
namespace driver {
class BLDriver;
}

namespace raw_driver {

struct RawDriverInfo {
  bool use_legacy_get_version = false;
};

class RawDriverBase {
  friend class driver::BLDriver;

 protected:
  RawDriverInfo info;

 public:
  virtual ~RawDriverBase() = default;

  virtual void ACK(TickType_t timeout = portMAX_DELAY) = 0;
  virtual void Send(OutboundData const &data) = 0;

  virtual std::vector<uint8_t> Recv(size_t length, bool resume = false) = 0;
  virtual void CommandHeader(uint8_t command) = 0;

  virtual void Sync() = 0;
};

}  // namespace raw_driver

}  // namespace stm32