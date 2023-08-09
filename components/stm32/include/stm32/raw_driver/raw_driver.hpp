#pragma once

#include "impl/base.hpp"
#include "impl/uart.hpp"

namespace stm32::raw_driver {

namespace auto_guesser {
template <typename DataLink>
class RawDriver {
  // No type declare
};

template <>
class RawDriver<connection::data_link::UART> {
 public:
  using type = impl::UART;
};
}  // namespace auto_guesser

template <typename DataLink>
using RawDriver = auto_guesser::RawDriver<DataLink>::type;
}  // namespace stm32::raw_driver