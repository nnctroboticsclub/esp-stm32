#include "./data_proxy_master.hpp"
#include <esp_log.h>
#include <cstring>

#include <data_proxy/bus.hpp>
#include <data_proxy/link.hpp>
#include <data_proxy/port.hpp>
#include <data_proxy/types/errno.hpp>

#include <stream/datalink/spi.hpp>
#include <stream/datalink/uart.hpp>
#include <stream/datalink/i2c_dev.hpp>
#include <i2c_cxx.hpp>

namespace esp_stm32::data_proxy {

//* Port
using SPIPort = HWPort<stream::datalink::SPIDevice>;
using I2CPort = HWPort<stream::datalink::I2CDevice>;

//* Bus
class SPIBus : public Bus {
  idf::SPIMaster master;

 public:
  ~SPIBus() override = default;
  inline explicit SPIBus(BusID bus_id, idf::SPINum spi_bus_id,
                         idf::MOSI const mosi, idf::MISO const miso,
                         idf::SCLK const sclk)
      : Bus(bus_id), master(spi_bus_id, mosi, miso, sclk) {}
  std::shared_ptr<Port> CreatePortImpl(uint8_t port_id,
                                       std::vector<uint8_t> const& _) override {
    auto port =
        std::make_shared<SPIPort>(port_id, this->master, (idf::CS)port_id);
    return port;
  }

  void HandleTscPort(const Packet& packet) final { throw NotImplemented(); }
  void HandleTscBus(const Packet& packet) final { throw NotImplemented(); }
};

class I2CBus : public Bus {
  idf::I2CMaster master;
  bool send_start_condition = true;
  bool send_stop_condition = true;

 public:
  ~I2CBus() override = default;
  inline explicit I2CBus(BusID bus_id, idf::I2CNumber i2c_bus_id,
                         idf::SDA_GPIO const sda, idf::SCL_GPIO const scl)
      : Bus(bus_id), master(i2c_bus_id, scl, sda, idf::Frequency::KHz(100)) {}

  std::shared_ptr<Port> CreatePortImpl(uint8_t port_id,
                                       std::vector<uint8_t> const& _) override {
    auto port = std::make_shared<I2CPort>(port_id, this->master,
                                          idf::I2CAddress(port_id));
    return port;
  }
  void HandleTscPort(const Packet& packet) final { throw NotImplemented(); }
  void HandleTscBus(const Packet& packet) final {
    auto number = packet.GetTscBusCommandNumber();

    switch (number) {
      case 8:
      case 9: {
        this->send_start_condition = number == 8;
        break;
      }
      case 10:
      case 11: {
        this->send_start_condition = number == 10;
        break;
      }
    }
  }
};

std::shared_ptr<Bus> ESPMaster::NewBus(Packet const& packet) {
  auto bus_type = static_cast<BusType>(packet.bus_id >> 6);
  int bus_id = packet.bus_id & 0x3f;

  using enum BusType;

  switch (bus_type) {
    case kI2C: {
      idf::SDA_GPIO sda(packet.data[0]);
      idf::SCL_GPIO scl(packet.data[1]);
      idf::I2CNumber i2c_number = ([](int bus_id) {
        switch (bus_id) {
          case 0:
            return idf::I2CNumber::I2C0();
          case 1:
            return idf::I2CNumber::I2C1();
          default:
            throw InvalidBusID();
        }
      })(bus_id);

      return std::make_shared<I2CBus>(packet.bus_id, i2c_number, sda, scl);
    }
    case kSPI:
      return nullptr;  // TODO(syoch): Impl this
    case kPipe:
      return nullptr;  // TODO(syoch): Impl this
    case kUART:
      return nullptr;  // TODO(syoch): Impl this
    default:
      return nullptr;
  }
}

}  // namespace esp_stm32::data_proxy