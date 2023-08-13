#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cinttypes>
#include <type_traits>

#include "../types/inbounddata.hpp"
#include "../types/outbounddata.hpp"

namespace stm32::raw_driver {

class RawDriverBase {
 public:
  virtual ~RawDriverBase() = default;

  virtual void ACK(TickType_t timeout = portMAX_DELAY) = 0;
  virtual void Send(OutboundData const &data) = 0;

  virtual void Recv(InboundData &data) = 0;
  virtual void CommandHeader(uint8_t command) = 0;

  virtual void Sync() = 0;
};

}  // namespace stm32::raw_driver
