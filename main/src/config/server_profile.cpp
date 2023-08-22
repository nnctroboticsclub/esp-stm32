#include "./server_profile.hpp"

namespace profile {
ServerProfile::ServerProfile(std::string const& ns)
    : nvs::Namespace(ns), ip(this, "ip"), port(this, "port") {}

}  // namespace profile