#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "libs/nvs_proxy.hpp"

#ifdef CONFIG_STM32_BOOTLOADER_DRIVER
#include "libs/stmbootloader.hpp"
#endif

#ifdef CONFIG_USE_NETWORK
#include "libs/wifi.hpp"
#endif

#ifdef CONFIG_USE_DATA_SERVER
#include "server.hpp"
#endif

#ifdef CONFIG_STM32_REMOTE_CONTROLLER_DRIVER
#include "libs/debugger_master.hpp"
#endif

namespace types {
union Ipv4 {
  uint32_t ip;
  uint8_t ip_bytes[4];
};
enum NetworkMode : uint8_t { None, STA, AP };
}  // namespace types

namespace nvs {
template <>
class Proxy<types::Ipv4> : public Proxy<uint32_t> {
 public:
  using Proxy<uint32_t>::Proxy;

  operator types::Ipv4() {
    types::Ipv4 value;
    value.ip = Proxy<uint32_t>::operator uint32_t();
    return value;
  }

  Proxy& operator=(types::Ipv4 value) {
    Proxy<uint32_t>::operator=(value.ip);
    return *this;
  }
};

template <>
class Proxy<types::NetworkMode> : public Proxy<uint8_t> {
 public:
  using Proxy<uint8_t>::Proxy;

  operator types::NetworkMode() {
    types::NetworkMode value;
    value = (types::NetworkMode)Proxy<uint8_t>::operator uint8_t();
    return value;
  }

  Proxy& operator=(types::NetworkMode value) {
    Proxy<uint8_t>::operator=((uint8_t)value);
    return *this;
  }
};
}  // namespace nvs

namespace config {

class NetworkProfile {  // a_nw%d
 public:
  nvs::Proxy<types::NetworkMode> mode;

  nvs::Proxy<char[20]> name;

  nvs::Proxy<char[20]> ssid;
  nvs::Proxy<char[20]> password;

  nvs::Proxy<char[20]> hostname;

  nvs::Proxy<types::Ipv4> ip;
  nvs::Proxy<types::Ipv4> subnet;
  nvs::Proxy<types::Ipv4> gateway;
  nvs::Proxy<types::Ipv4> dns;

 public:
  NetworkProfile(nvs::Namespace* ns);
};

class ServerProfile {
 public:
  nvs::Proxy<types::Ipv4> ip;
  nvs::Proxy<uint16_t> port;

  ServerProfile(nvs::Namespace* ns);
};

class STM32BootLoaderProfile {
 public:
  nvs::Proxy<uint8_t> reset;
  nvs::Proxy<uint8_t> boot0;
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<uint8_t> uart_tx;
  nvs::Proxy<uint8_t> uart_rx;

  STM32BootLoaderProfile(nvs::Namespace* ns);
};

class STM32RemoteControllerProfile {
 public:
  nvs::Proxy<uint8_t> uart_port;
  nvs::Proxy<uint8_t> uart_tx;
  nvs::Proxy<uint8_t> uart_rx;

  STM32RemoteControllerProfile(nvs::Namespace* ns);
};

class Config {
 public:
  NetworkProfile network_profiles[5];
  nvs::Proxy<uint8_t> active_network_profile;

  ServerProfile server_profile;

  STM32BootLoaderProfile stm32_bootloader_profile;
  STM32RemoteControllerProfile stm32_remote_controller_profile;

 private:
  Config();

 public:
  static Config* CreateDefaultConfig();
  static inline Config* GetInstance() {
    static Config* instance = nullptr;
    if (instance == nullptr) {
      instance = new Config();
    }
    return instance;
  }
};

#ifdef CONFIG_STM32_BOOTLOADER_DRIVER
extern STMBootLoader loader;
#endif

#ifdef CONFIG_STM32_REMOTE_CONTROLLER_DRIVER
extern DebuggerMaster debugger;
#endif

#ifdef CONFIG_USE_NETWORK
extern app::Wifi network;
#endif

#ifdef CONFIG_USE_DATA_SERVER
extern Server server;
#endif
}  // namespace config

#endif