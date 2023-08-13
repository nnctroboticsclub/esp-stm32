#pragma once

#include <esp_log.h>

#include <memory>
#include <vector>

#include <connection/data_link/uart.hpp>

#include "../types/error.hpp"
#include "../types/inbounddata.hpp"
#include "../types/outbounddata.hpp"
#include "./base.hpp"

namespace stm32::raw_driver::impl {
using UartDataLink = connection::data_link::UART;

class UART : public RawDriverBase {
  static constexpr const char *TAG = "RawDriver<UART>";

  std::shared_ptr<UartDataLink> device;

 public:
  explicit UART(std::shared_ptr<connection::data_link::UART> device);
  ~UART() = default;

  void ACK(TickType_t timeout = portMAX_DELAY) override;

  void Send(const OutboundData &data) override;
  void Recv(InboundData &data) override;

  void CommandHeader(uint8_t command) override;

  void Sync() override;
};
}  // namespace stm32::raw_driver::impl
