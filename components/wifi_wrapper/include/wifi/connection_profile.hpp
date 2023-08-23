#pragma once

#include <string>

namespace wifi {
struct WifiConnectionProfile {
  wifi_auth_mode_t auth_mode;
  const std::string ssid;
  const std::string password;
  const std::string user;
  const std::string id;
};
}  // namespace wifi