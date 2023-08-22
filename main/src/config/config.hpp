#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <memory>
#include <vector>

#include <connection/data_link/spi.hpp>
#include <connection/data_link/uart.hpp>

#include "../libs/nvs_proxy.hpp"

#include <wifi.hpp>
#include "../server.hpp"
#include <debugger_master.hpp>
#include <stm32.hpp>

#include "spibus.hpp"
#include "uartport.hpp"
#include "network_profile.hpp"
#include "s32rc.hpp"
#include "server_profile.hpp"

namespace profile {

class Master : public nvs::Namespace {
 public:
  nvs::Proxy<uint8_t> spi_buses;
  nvs::Proxy<uint8_t> uart_buses;
  nvs::Proxy<uint8_t> s32_count;
  nvs::Proxy<uint8_t> s32bl_count;
  nvs::Proxy<uint8_t> s32rc_count;
  nvs::Proxy<uint8_t> nw_count;
  nvs::Proxy<uint8_t> srv_count;
  nvs::Proxy<uint8_t> active_net;
  nvs::Proxy<uint8_t> active_srv;
  nvs::Proxy<uint8_t> primary_s32bl;
  nvs::Proxy<uint8_t> primary_s32rc;

  explicit Master(std::string const& ns)
      : nvs::Namespace(ns),
        spi_buses(this, "cds"),
        uart_buses(this, "cdu"),
        s32_count(this, "cs3"),
        s32bl_count(this, "csb"),
        s32rc_count(this, "csr"),
        nw_count(this, "cn"),
        srv_count(this, "cfs"),
        active_net(this, "an"),
        active_srv(this, "afs"),
        primary_s32bl(this, "asb"),
        primary_s32rc(this, "asr") {}
};



class STM32BL : public nvs::Namespace {
 public:
  nvs::Proxy<uint8_t> bus_type;
  nvs::Proxy<uint8_t> bus_port;

  explicit STM32BL(std::string const& ns)
      : nvs::Namespace(ns),
        bus_type(this, "bus_type"),
        bus_port(this, "bus_port") {}
};

class STM32RC : public nvs::Namespace {
 public:
  nvs::Proxy<uint8_t> bus_type;
  nvs::Proxy<uint8_t> bus_port;

  explicit STM32RC(std::string const& ns)
      : nvs::Namespace(ns),
        bus_type(this, "bus_type"),
        bus_port(this, "bus_port") {}
};

class STM32 : public nvs::Namespace {
  std::shared_ptr<stm32::STM32> stm32;
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<uint8_t> bl_id;
  nvs::Proxy<uint8_t> rc_id;
 public:

  explicit STM32(std::string const& ns)
      : nvs::Namespace(ns),
        reset(this, "reset"),
        boot0(this, "boot0"),
        bl_id(this, "bid"),
        rc_id(this, "rid") {}
};

}  // namespace profile

namespace config {

class Config {
  using Master = profile::Master;
  using SPIBus = profile::SPIBus;
  using UARTPort = profile::UartPort;
  using STM32 = profile::STM32;
  using STM32BL = profile::STM32BL;
  using STM32RC = profile::STM32RC;
  using NetworkProfile = profile::NetworkProfile;
  using ServerProfile = profile::ServerProfile;

  static std::shared_ptr<Config> instance;
  Config() {
    using nvs::LoadNamespaces;
    this->spi_buses = LoadNamespaces<SPIBus>("a_cs", (uint8_t)master.spi_buses);
    this->uart_buses =
        LoadNamespaces<UARTPort>("a_cu", (uint8_t)master.uart_buses);
    this->stm32 = LoadNamespaces<STM32>("a_s3", (uint8_t)master.s32_count);
    this->stm32_bootloader =
        LoadNamespaces<STM32BL>("a_sb", (uint8_t)master.s32bl_count);
    this->stm32_remote_controller =
        LoadNamespaces<STM32RC>("a_sr", (uint8_t)master.s32rc_count);
    this->network_profiles =
        LoadNamespaces<NetworkProfile>("a_nw", (uint8_t)master.nw_count);
    this->server_profiles =
        LoadNamespaces<ServerProfile>("a_sr", (uint8_t)master.srv_count);
  }
  ~Config();

  friend std::shared_ptr<Config>;

 public:
  Master master{"mas"};
  std::vector<SPIBus> spi_buses;
  std::vector<UARTPort> uart_buses;
  std::vector<STM32> stm32;
  std::vector<STM32BL> stm32_bootloader;
  std::vector<STM32RC> stm32_remote_controller;
  std::vector<NetworkProfile> network_profiles;
  std::vector<ServerProfile> server_profiles;

 public:
  static std::shared_ptr<Config> GetInstance() {
    if (instance == nullptr) {
      instance = std::make_shared<Config>();
    }
    return instance;
  }

  static STM32
};

extern Server server;
extern wifi::Wifi network;
}  // namespace config

#endif