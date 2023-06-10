#include "config.hpp"
#include "../init/init.hpp"
#include <nvs.h>

using namespace profile;

namespace config {

Config::Config()
    : network_profiles{
          NetworkProfile(new nvs::Namespace("a_nw0")),
          NetworkProfile(new nvs::Namespace("a_nw1")),
          NetworkProfile(new nvs::Namespace("a_nw2")),
          NetworkProfile(new nvs::Namespace("a_nw3")),
          NetworkProfile(new nvs::Namespace("a_nw4")),
      },
      active_network_profile(new nvs::Namespace("a_conf"), "active_nw"),
      server_profile(new nvs::Namespace("a_srv")),
      stm32_bootloader_profile(new nvs::Namespace("a_s32bl")),
      stm32_remote_controller_profile(new nvs::Namespace("a_s32rc")) {}

// DebuggerMaster debugger(UART_NUM_2, 11, 12);

app::Wifi network;

Server server;
}  // namespace config