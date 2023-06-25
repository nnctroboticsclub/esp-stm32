#include <wifi/authmode_kind.hpp>
namespace wifi {
AuthModeKind GetAuthModeKind(wifi_auth_mode_t authmode) {
  switch (authmode) {
    case WIFI_AUTH_OPEN:
      return AuthModeKind::kOpen;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return AuthModeKind::kPassAndUser;
    case WIFI_AUTH_WEP:
    case WIFI_AUTH_WPA_PSK:
    case WIFI_AUTH_WPA2_PSK:
    case WIFI_AUTH_WPA_WPA2_PSK:
    case WIFI_AUTH_WPA3_PSK:
    case WIFI_AUTH_WPA2_WPA3_PSK:
    case WIFI_AUTH_WAPI_PSK:
    case WIFI_AUTH_OWE:
    case WIFI_AUTH_MAX:
      return AuthModeKind::kPassOnly;
    default:
      return AuthModeKind::kPassOnly;
  }
}
}  // namespace wifi