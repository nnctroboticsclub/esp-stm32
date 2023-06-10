#include "server_profile.hpp"

namespace profile {
ServerProfile::ServerProfile(nvs::Namespace* ns)
    : ip(ns, "ip"), port(ns, "port") {}
}  // namespace profile