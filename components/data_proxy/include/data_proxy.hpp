#pragma once

// Version: 1.0.0

#include <esp_log.h>

#include <vector>
#include <cinttypes>
#include <memory>
#include <cstring>
#include <map>
#include <utility>

#include <stream/base.hpp>
#include <stream/datalink/spi.hpp>
#include <stream/datalink/uart.hpp>
#include <stream/datalink/i2c_dev.hpp>
#include <i2c_cxx.hpp>

namespace data_proxy {

enum class BusType { SPI, I2C, UART, Pipe };

using PortID = uint8_t;
using BusID = uint8_t;

inline BusType GetBusType(BusID bus_id) {
  return static_cast<BusType>(bus_id >> 6);
}

enum class CommandKind {
  kSession = 0,
  kIO,
  kGeneral,
  kReserved1,
  kReserved2,
  kReserved3,
  kReserved4,
  kReserved5,
};
enum class Command : uint8_t {
  kBusClose = 0x00,
  kBusNewSPI = 0x01,
  kBusNewI2C = 0x02,
  kBusNewNotify = 0x08,
  kBusDelNotify = 0x09,

  kPortClose = 0x10,
  kPortNew = 0x11,
  kPortNewNotify = 0x18,
  kPortDelNotify = 0x19,
  kPortErrNotify = 0x1a,

  kIoPortWrite = 0x30,
  kIoPortRead = 0x31,
  kIoPortAvail = 0x32,
  kIoPortDataResponse = 0x38,
  kIoPortLengthResponse = 0x39,

  kGeneralRequestBusList = 0x40,
  kGeneralVersion = 0x47,
  kGeneralBusList = 0x48,
  kGeneralUnrecoverableError = 0x49,
  kGeneralRecoverableError = 0x4a,
  kGeneralRequestPortList = 0x50,
  kGeneralPortList = 0x58,
};

struct Packet {
  union {
    Command command;
    uint8_t raw_command;
  };
  uint8_t bus_id;
  uint8_t port_id;
  uint8_t optional;

  uint32_t data_size;
  const uint8_t *data;

  ~Packet() {}

  void Debug() const {
    ESP_LOGI("Packet", "Structure");
    ESP_LOGI("Packet", "  Command: %d", raw_command);
    ESP_LOGI("Packet", "  Bus ID: %d", bus_id);
    ESP_LOGI("Packet", "  Port ID: %d", port_id);
    ESP_LOGI("Packet", "  Optional: %d", optional);
    for (size_t i = 0; i < data_size; i++) {
      ESP_LOGI("Packet", "  Data[%d]: %d", i, data[i]);
    }
  }
};

enum class Errno : uint8_t {
  kUnknownCommand,
  kUnknownBus,
  kUnknownPort,
  kUnknownError,
  kMalformedPacket,
  kMalformedData,
  kInvalidBusID,
  kInvalidPortID,
};

class Link {
 public:
  static constexpr uint8_t kPacketHeaderSize =
      sizeof(Packet) - sizeof(uint32_t);

 private:
  using T = std::shared_ptr<stream::RecvAndSend>;
  T link;

 public:
  explicit Link(T link);

  void SendPacket(const Packet &packet) const;

  Packet RecvPacket() const;
};

class Port : public stream::RecvAndSend {
  PortID id;

 public:
  inline explicit Port(PortID id) : id(id) {}

  inline PortID GetID() const { return id; }
};

class Bus {
  std::map<PortID, std::shared_ptr<Port>> ports;
  virtual std::shared_ptr<Port> CreatePortImpl(uint8_t port_id) = 0;

 public:
  virtual ~Bus() = default;
  std::shared_ptr<Port> GetPort(PortID port_id, bool auto_generate);
};

class Proxy {
  class Impl;
  std::shared_ptr<Impl> impl;

 public:
  explicit Proxy(Link link);

  std::shared_ptr<Bus> GetBus(BusID bus_id);

  std::thread Start();
};

}  // namespace data_proxy