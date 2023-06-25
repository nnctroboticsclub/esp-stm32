#pragma once

namespace wifi {
struct WifiConnectionProfile {
  wifi_auth_mode_t auth_mode;
  const char* ssid;
  const char* password;
  const char* user;
  const char* id;
};
}  // namespace wifi