#pragma once

#include <stdexcept>
#include <string>

namespace stm32::driver {
class NotImplemented : public std::runtime_error {
 public:
  explicit NotImplemented(const std::string &message = "Unknown Error")
      : std::runtime_error("Not Implemented: " + message) {}
};
}  // namespace stm32::driver
