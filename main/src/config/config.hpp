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
#include <debugger_master.hpp>
#include <stm32.hpp>
#include "types/bus_type.hpp"

#include "./spibus.hpp"
#include "./uartport.hpp"
#include "./network_profile.hpp"
#include "./s32rc.hpp"
#include "./server_profile.hpp"

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

  nvs::Proxy<bool> initialised;

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
        primary_s32rc(this, "asr"),
        initialised(this, "init") {}
};

class STM32BL : public nvs::Namespace {
 public:
  nvs::Proxy<uint8_t> id;
  nvs::Proxy<types::BusType> bus_type;
  nvs::Proxy<uint8_t> bus_port;
  nvs::Proxy<uint8_t> cs;

  explicit STM32BL(std::string const& ns)
      : nvs::Namespace(ns),
        id(this, "id"),
        bus_type(this, "bus_type"),
        bus_port(this, "bus_port"),
        cs(this, "cs") {}

  std::shared_ptr<stm32::driver::BLDriver> GetDriver() const;

  inline uint8_t GetID() const { return id; }

  inline STM32BL& SetID(uint8_t id_) {
    this->id = id_;
    return *this;
  }

  inline STM32BL& SetBusType(types::BusType type) {
    this->bus_type = type;
    return *this;
  }

  inline STM32BL& SetBusPort(uint8_t port) {
    this->bus_port = port;
    return *this;
  }

  inline STM32BL& SetCS(uint8_t cs_) {
    this->cs = cs_;
    return *this;
  }
};

class STM32RC : public nvs::Namespace {
 public:
  nvs::Proxy<types::BusType> bus_type;
  nvs::Proxy<uint8_t> bus_port;

  explicit STM32RC(std::string const& ns)
      : nvs::Namespace(ns),
        bus_type(this, "bus_type"),
        bus_port(this, "bus_port") {}
};

class STM32 : public nvs::Namespace {
  std::shared_ptr<stm32::STM32> stm32 = nullptr;

  nvs::Proxy<uint8_t> id;
  nvs::Proxy<gpio_num_t> reset;
  nvs::Proxy<gpio_num_t> boot0;
  nvs::Proxy<uint8_t> bl_id;
  nvs::Proxy<uint8_t> rc_id;

 public:
  explicit STM32(std::string const& ns)
      : nvs::Namespace(ns),
        id(this, "id"),
        reset(this, "reset"),
        boot0(this, "boot0"),
        bl_id(this, "bid"),
        rc_id(this, "rid") {}

  std::shared_ptr<stm32::STM32> Get();

  inline uint8_t GetID() const { return id; }

  inline STM32& SetID(uint8_t id_) {
    this->id = id_;
    return *this;
  }

  inline STM32& SetReset(gpio_num_t pin) {
    reset = pin;
    return *this;
  }

  inline STM32& SetBoot0(gpio_num_t pin) {
    boot0 = pin;
    return *this;
  }

  inline STM32& SetBL_ID(uint8_t id) {
    bl_id = id;
    return *this;
  }

  inline STM32& SetRC_ID(uint8_t id) {
    rc_id = id;
    return *this;
  }
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

  static Config instance;

 public:
  Master master{"mas"};
  nvs::Namespaces<SPIBus, "a_cs"> spi_buses;
  nvs::Namespaces<UARTPort, "a_cu"> uart_buses;
  nvs::Namespaces<STM32, "a_s3"> stm32;
  nvs::Namespaces<STM32BL, "a_sb"> stm32_bootloader;
  nvs::Namespaces<STM32RC, "a_sr"> stm32_remote_controller;
  nvs::Namespaces<NetworkProfile, "a_nw"> network_profiles;
  nvs::Namespaces<ServerProfile, "a_sr"> server_profiles;

 private:
  Config()
      : spi_buses((uint8_t)master.spi_buses),
        uart_buses((uint8_t)master.uart_buses),
        stm32((uint8_t)master.s32_count),
        stm32_bootloader((uint8_t)master.s32bl_count),
        stm32_remote_controller((uint8_t)master.s32rc_count),
        network_profiles((uint8_t)master.nw_count),
        server_profiles((uint8_t)master.srv_count) {}
  ~Config() = default;

  friend std::shared_ptr<Config>;

 public:
  static Config& GetInstance() { return instance; }

  //* Getter

  static std::shared_ptr<idf::SPIMaster> GetSPIBus(uint8_t port) {
    for (auto& bus : instance.spi_buses) {
      if (bus.GetPort() == port) {
        return bus.GetDevice();
      }
    }
    return nullptr;
  }

  static std::shared_ptr<connection::data_link::UART> GetUARTPort(uint8_t id) {
    for (auto& bus : instance.uart_buses) {
      if (bus.GetPort() == id) {
        return bus.GetDevice();
      }
    }
    return nullptr;
  }

  static std::shared_ptr<stm32::driver::BLDriver> GetSTM32BL(uint8_t id) {
    for (auto& s32 : instance.stm32_bootloader) {
      if (s32.GetID() == id) {
        return s32.GetDriver();
      }
    }
    return nullptr;
  }

  static std::shared_ptr<NetworkProfile> GetActiveNetworkProfile() {
    return std::make_shared<NetworkProfile>(
        instance.network_profiles[(uint8_t)instance.master.active_net]);
  }

  static std::shared_ptr<stm32::STM32> GetSTM32(uint8_t id) {
    for (auto& s32 : instance.stm32) {
      if (s32.GetID() == id) {
        return s32.Get();
      }
    }
    return nullptr;
  }

  static std::shared_ptr<stm32::STM32> GetPrimarySTM32() {
    return GetSTM32(instance.master.primary_s32bl);
  }

  //* Set

  static void SetActiveSTM32(uint8_t id) { instance.master.primary_s32bl = id; }

  //* New

  static SPIBus& NewSPIBus() { return instance.spi_buses.New(); }

  static UARTPort& NewUARTPort() { return instance.uart_buses.New(); }

  static STM32& NewSTM32() { return instance.stm32.New(); }

  static STM32BL& NewSTM32BL() { return instance.stm32_bootloader.New(); }

  static STM32RC& NewSTM32RC() {
    return instance.stm32_remote_controller.New();
  }

  static NetworkProfile& NewNetworkProfile() {
    return instance.network_profiles.New();
  }

  static ServerProfile& NewServerProfile() {
    return instance.server_profiles.New();
  }
};

// extern Server server;
extern wifi::Wifi network;
}  // namespace config

#endif