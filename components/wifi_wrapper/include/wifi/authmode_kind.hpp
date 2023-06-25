#pragma once

#include <esp_wifi.h>

namespace wifi {

enum class AuthModeKind { kOpen, kPassOnly, kPassAndUser };
AuthModeKind GetAuthModeKind(wifi_auth_mode_t authmode);

}  // namespace wifi