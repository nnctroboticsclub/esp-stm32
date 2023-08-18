#pragma once

#include "impl/base.hpp"
#include "impl/uart.hpp"
#include "impl/spi.hpp"

namespace stm32::raw_driver {
using UartRawDriver = impl::UART;
using SPIRawDriver = impl::SPI;
}  // namespace stm32::raw_driver