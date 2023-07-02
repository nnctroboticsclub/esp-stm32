#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "../libs/nvs_proxy.hpp"

#include <wifi.hpp>
#include "../server.hpp"
#include <debugger_master.hpp>

#include "network_profile.hpp"
#include "s32bl.hpp"
#include "s32rc.hpp"
#include "server_profile.hpp"

namespace config {

class Config {
 public:
  profile::NetworkProfile network_profiles[5];
  nvs::Proxy<uint8_t> active_network_profile;

  profile::ServerProfile server_profile;

  profile::STM32BLProfileInterface* stm32_bootloader_profile;
  profile::STM32RemoteControllerProfile stm32_remote_controller_profile;

 private:
  Config();
  ~Config();

 public:
  static Config* CreateDefaultConfig();
  static inline Config* GetInstance() {
    auto& instance = *GetInstancePtr();
    if (instance == nullptr) {
      instance = new Config();
    }
    return instance;
  }
  static inline void DeleteInstance() {  // Not recommended to use (all nvs
                                         // namespaces will be committed)
    auto& instance = *GetInstancePtr();
    delete instance;
    instance = nullptr;
  }

  static connection::application::STM32BootLoader* GetSTM32BootLoader() {
    return Config::GetInstance()->stm32_bootloader_profile->GetLoader();
  }

  static DebuggerMaster* GetDebuggerMaster() {
    return Config::GetInstance()
        ->stm32_remote_controller_profile.GetDebuggerMaster();
  }

 private:
  static inline Config** GetInstancePtr() {
    static Config* instance = nullptr;
    return &instance;
  }
};

extern Server server;
extern wifi::Wifi network;
}  // namespace config

#endif