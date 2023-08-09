#pragma once

#include <stdexcept>

namespace stm32::raw_driver {
class ConnectionDisrupted : public std::runtime_error {
 public:
  explicit ConnectionDisrupted(const std::string &message = "Unknown Error")
      : std::runtime_error("Connection Disrupted: " + message) {}
};
class ACKFailed : public ConnectionDisrupted {
 public:
  ACKFailed() : ConnectionDisrupted(": ACK Failed") {}
};

}  // namespace stm32::raw_driver
