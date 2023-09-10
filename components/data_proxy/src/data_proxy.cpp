#include <data_proxy.hpp>

namespace data_proxy {

namespace {
template <typename ToCheck, std::size_t ExpectedSize,
          std::size_t RealSize = sizeof(ToCheck)>
void check_size() {
  static_assert(ExpectedSize == RealSize, "Size is off!");
}
}  // namespace

Link::Link(T link) : link(link) { check_size<Command, 1>(); }

void Link::SendPacket(const Packet& packet) const {
  const Packet* packet_ptr = &packet;

  // Send header
  {
    const std::vector<uint8_t> buffer(
        reinterpret_cast<const uint8_t*>(packet_ptr),
        reinterpret_cast<const uint8_t*>(packet_ptr) + kPacketHeaderSize);
    this->link->Send(buffer);
  }

  // Send data
  if (packet_ptr->data_size) {
    std::vector<uint8_t> buffer(packet_ptr->data,
                                packet_ptr->data + packet_ptr->data_size);
    this->link->SendU32(packet_ptr->data_size);
    this->link->Send(buffer);
  }
}

Packet Link::RecvPacket() const {
  // ACK
  do {
    auto ch = this->link->RecvChar();
    if (ch == 0xa5) {
      this->link->SendChar(0x5A);
      break;
    } else {
      this->link->SendChar(0xFE);
    }
  } while (true);

  Packet packet{.raw_command = 0,
                .bus_id = 0,
                .port_id = 0,
                .optional = 0,
                .data_size = 0,
                .data = nullptr};

  // Recv header
  {
    std::vector<uint8_t> buffer(kPacketHeaderSize - sizeof(uint32_t));
    this->link->Recv(buffer);

    packet.raw_command = buffer[0];
    packet.bus_id = buffer[1];
    packet.port_id = buffer[2];
    packet.optional = buffer[3];
  }

  // Recv data
  packet.data_size = this->link->RecvU32();
  if (packet.data_size) {
    std::vector<uint8_t> buffer(packet.data_size, 0);
    this->link->Recv(buffer);

    auto raw_buf = new uint8_t[packet.data_size];
    memcpy(raw_buf, buffer.data(), packet.data_size);

    packet.data = raw_buf;
  }

  return packet;
}

//* Bus
std::shared_ptr<Port> Bus::GetPort(PortID port_id, bool auto_generate) {
  auto it = this->ports.find(port_id);

  if (it != this->ports.end()) {
    return it->second;
  }

  if (!auto_generate) {
    return nullptr;
  }

  auto port = CreatePortImpl(port_id);
  this->ports[port_id] = port;
  return port;
}

//* SPIBus
std::shared_ptr<Port> SPIBus::CreatePortImpl(uint8_t port_id) {
  auto port =
      std::make_shared<SPIPort>(port_id, this->master, (idf::CS)port_id);
  return port;
}

//* I2CBus
std::shared_ptr<Port> I2CBus::CreatePortImpl(uint8_t port_id) {
  auto port = std::make_shared<I2CPort>(port_id, this->master,
                                        idf::I2CAddress(port_id));
  return port;
}

//* Proxy
class Proxy::Impl {
  class HandlerError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
  };

  class InvalidBusID : public HandlerError {
   public:
    InvalidBusID() : HandlerError("Invalid bus ID") {}
  };

  class UnknownBus : public HandlerError {
   public:
    UnknownBus() : HandlerError("Unknown bus") {}
  };

  class UnknownPort : public HandlerError {
   public:
    UnknownPort() : HandlerError("Unknown port") {}
  };

  static constexpr const char* TAG = "Proxy[Impl]";
  static void (Impl::*handlers[256])(Packet const&);

  Link link;
  std::map<BusID, std::shared_ptr<Bus>> buses;

  void NotifyNewBus(uint8_t bus_id) {
    ESP_LOGI(TAG, "Notifying new bus %d", bus_id);
    Packet response;
    response.command = Command::kBusNewNotify;
    response.bus_id = bus_id;
    response.port_id = 0;
    response.optional = 0;
    response.data_size = 0;
    response.data = nullptr;
    this->link.SendPacket(response);
  }

  void NotifyNewPort(uint8_t bus_id, uint8_t port_id) {
    ESP_LOGI(TAG, "Notifying new port %d on bus %d", port_id, bus_id);
    Packet response;
    response.command = Command::kPortNewNotify;
    response.bus_id = bus_id;
    response.port_id = port_id;
    response.optional = 0;
    response.data_size = 0;
    response.data = nullptr;
    this->link.SendPacket(response);
  }

  void NotifyIOPortLength(uint32_t length) {
    uint8_t buffer[sizeof(uint32_t)];
    buffer[0] = (length >> 24) & 0xff;
    buffer[1] = (length >> 16) & 0xff;
    buffer[2] = (length >> 8) & 0xff;
    buffer[3] = (length >> 0) & 0xff;

    Packet response;
    response.command = Command::kIoPortLengthResponse;
    response.bus_id = 0;
    response.port_id = 0;
    response.optional = 0;
    response.data_size = sizeof(uint32_t);
    response.data = buffer;
    this->link.SendPacket(response);
  }

  void NotifyIOPortData(std::vector<uint8_t> const& data) {
    Packet response;
    response.command = Command::kIoPortDataResponse;
    response.bus_id = 0;
    response.port_id = 0;
    response.optional = 0;

    response.data_size = data.size();
    response.data = data.data();
    this->link.SendPacket(response);
  }

  void HandleNotImplemented(Packet const& packet) {
    ESP_LOGI(TAG, "Not implemented command: %d", packet.raw_command);
    Packet response;
    response.command = Command::kGeneralRecoverableError;
    response.bus_id = packet.bus_id;
    response.port_id = packet.port_id;
    response.optional = packet.optional;
    response.data_size = 0;
    response.data = nullptr;
    this->link.SendPacket(response);
  }

  void HandleNewSPIBus(Packet const& packet) {
    using enum CommandKind;

    auto& data = packet.data;

    auto bus_id = packet.bus_id;

    ESP_LOGI(TAG, "Creating SPI bus %d", bus_id);
    ESP_LOGI(TAG, "  - MOSI: %d", data[0]);
    ESP_LOGI(TAG, "  - MISO: %d", data[1]);
    ESP_LOGI(TAG, "  - SCLK: %d", data[2]);

    auto bus = std::make_shared<SPIBus>(  //
        static_cast<idf::SPINum>(bus_id), static_cast<idf::MOSI const>(data[0]),
        static_cast<idf::MISO const>(data[1]),
        static_cast<idf::SCLK const>(data[2]));
    buses[bus_id] = bus;

    this->NotifyNewBus(bus_id);
  }

  void HandlePortNew(Packet const& packet) {
    using enum CommandKind;

    auto bus_id = packet.bus_id;
    auto port_id = packet.port_id;

    ESP_LOGI(TAG, "Creating port %d on bus %d", port_id, bus_id);

    if (!this->buses.contains(bus_id)) {
      ESP_LOGE(TAG, "Unknown bus: %d", bus_id);
      throw UnknownBus();
    }
    auto bus = this->buses[bus_id];

    auto port = bus->GetPort(port_id, true);
    if (port) {
      ESP_LOGW(TAG, "Port already exists: %d", port_id);
    }

    this->NotifyNewPort(bus_id, port_id);
  }

  void HandlePortWrite(Packet const& packet) {
    if (!this->buses.contains(packet.bus_id)) {
      ESP_LOGE(TAG, "Unknown bus: %d", packet.bus_id);
      throw UnknownBus();
    }
    auto bus = this->buses[packet.bus_id];

    auto port = bus->GetPort(packet.port_id, false);
    if (!port) {
      ESP_LOGE(TAG, "Unknown port: %d", packet.port_id);
      throw UnknownPort();
    }

    std::vector<uint8_t> data(packet.data, packet.data + packet.data_size);
    port->Send(data);

    this->NotifyIOPortLength(data.size());
  }

  void HandlePortRead(Packet const& packet) {
    if (!this->buses.contains(packet.bus_id)) {
      ESP_LOGE(TAG, "Unknown bus: %d", packet.bus_id);
      throw UnknownBus();
    }
    auto bus = this->buses[packet.bus_id];

    auto port = bus->GetPort(packet.port_id, false);
    if (!port) {
      ESP_LOGE(TAG, "Unknown port: %d", packet.port_id);
      throw UnknownPort();
    }

    std::vector<uint8_t> data(packet.data_size, 0);
    port->Recv(data);

    this->NotifyIOPortData(data);
  }

  void HandleNewI2CBus(Packet const& packet) {
    using enum CommandKind;

    auto& data = packet.data;

    auto bus_id = packet.bus_id;

    idf::I2CNumber i2c_number = ([bus_id]() {
      if (bus_id == 0) {
        return idf::I2CNumber::I2C0();
      } else if (bus_id == 1) {
        return idf::I2CNumber::I2C1();
      } else {
        throw InvalidBusID();
      }
    })();

    ESP_LOGI(TAG, "Creating I2C bus %d", bus_id);
    ESP_LOGI(TAG, "  - SDA: %d", data[0]);
    ESP_LOGI(TAG, "  - SCL: %d", data[1]);

    auto bus = std::make_shared<I2CBus>(  //
        i2c_number, static_cast<idf::SDA_GPIO const>(data[0]),
        static_cast<idf::SCL_GPIO const>(data[1]));
    buses[bus_id] = bus;

    this->NotifyNewBus(bus_id);
  }

 public:
  Impl(Link link) : link(link) {
    using enum Command;

    for (auto& handler : handlers) {
      handler = &Impl::HandleNotImplemented;
    }
    handlers[(uint8_t)kBusNewSPI] = &Impl::HandleNewSPIBus;
    handlers[(uint8_t)kBusNewI2C] = &Impl::HandleNewI2CBus;
    handlers[(uint8_t)kPortNew] = &Impl::HandlePortNew;
    handlers[(uint8_t)kIoPortWrite] = &Impl::HandlePortWrite;
    handlers[(uint8_t)kIoPortRead] = &Impl::HandlePortRead;
  }

  [[noreturn]] void Thread() {
    while (true) {
      auto packet = this->link.RecvPacket();
      auto handler = handlers[packet.raw_command];
      try {
        (this->*handler)(packet);
      } catch (InvalidBusID&) {
        uint8_t buf[1] = {static_cast<uint8_t>(Errno::kInvalidBusID)};

        Packet response;
        response.command = Command::kGeneralRecoverableError;
        response.bus_id = packet.bus_id;
        response.port_id = packet.port_id;
        response.optional = packet.optional;
        response.data_size = 1;
        response.data = buf;
        this->link.SendPacket(response);
      } catch (UnknownBus&) {
        uint8_t buf[1] = {static_cast<uint8_t>(Errno::kUnknownBus)};

        Packet response;
        response.command = Command::kGeneralRecoverableError;
        response.bus_id = packet.bus_id;
        response.port_id = packet.port_id;
        response.optional = packet.optional;
        response.data_size = 1;
        response.data = buf;
        this->link.SendPacket(response);
      } catch (UnknownPort&) {
        uint8_t buf[1] = {static_cast<uint8_t>(Errno::kUnknownPort)};

        Packet response;
        response.command = Command::kGeneralRecoverableError;
        response.bus_id = packet.bus_id;
        response.port_id = packet.port_id;
        response.optional = packet.optional;
        response.data_size = 1;
        response.data = buf;
        this->link.SendPacket(response);
      }

      delete[] packet.data;
    }
  }
};

void (Proxy::Impl::*Proxy::Impl::handlers[256])(Packet const&);

Proxy::Proxy(Link link) : impl(std::make_unique<Impl>(link)) {}

std::thread Proxy::Start() {
  std::thread t([this]() { this->impl->Thread(); });
  return t;
}
}  // namespace data_proxy