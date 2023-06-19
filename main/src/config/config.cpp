#include "config.hpp"
#include "../init/init.hpp"
#include <nvs.h>

using namespace profile;

namespace config {

Config::Config()
    : network_profiles{
          NetworkProfile(nvs::SharedNamespace("a_nw0")),
          NetworkProfile(nvs::SharedNamespace("a_nw1")),
          NetworkProfile(nvs::SharedNamespace("a_nw2")),
          NetworkProfile(nvs::SharedNamespace("a_nw3")),
          NetworkProfile(nvs::SharedNamespace("a_nw4")),
      },
      active_network_profile(nvs::SharedNamespace("a_conf"), "active_nw"),
      server_profile(nvs::SharedNamespace("a_srv")),
      stm32_remote_controller_profile(nvs::SharedNamespace("a_s32rc")) {
  this->stm32_bootloader_profile =
      LoadSTM32BootLoaderProfile(nvs::SharedNamespace("a_s32bl"));
}

Config::~Config() { delete this->stm32_bootloader_profile; }

Server server;
app::Wifi network;
}  // namespace config