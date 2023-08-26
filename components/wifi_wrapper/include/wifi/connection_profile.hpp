#pragma once

#include <string>

namespace wifi {
struct WifiConnectionProfile {
  wifi_auth_mode_t auth_mode;
  std::string ssid;
  std::string password;
  std::string user;
  std::string id;
};
}  // namespace wifi