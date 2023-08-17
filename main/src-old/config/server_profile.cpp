#include "server_profile.hpp"

namespace profile {
ServerProfile::ServerProfile(nvs::SharedNamespace ns)
    : ns(ns), ip(ns, "ip"), port(ns, "port") {}

void ServerProfile::Save() {
    ns->Commit();
}
}  // namespace profile