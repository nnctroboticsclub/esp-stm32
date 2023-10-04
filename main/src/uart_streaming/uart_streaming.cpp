#include "./uart_streaming.hpp"

#include <vector>

#include <lwip/sockets.h>

#include "../init/init.hpp"

namespace {

bool socket_initialized = false;
int socket = -1;

void Init() {
  if (!socket_initialized) {
    init::init_wifi();
    socket = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket < 0) {
      throw std::runtime_error("Failed to create socket");
    }
    socket_initialized = true;
  }
}

void Broadcast_(std::vector<uint8_t> const &buf) {
  Init();
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9088);
  addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

  auto ret = lwip_sendto(socket, buf.data(), buf.size(), 0,
                         (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    throw std::runtime_error("Failed to send data");
  }
}

void BroadcastLog_(uint16_t id, std::vector<uint8_t> const &buf) {
  std::vector<uint8_t> header(8);
  header[0] = 0x00;
  header[1] = 0x00;  // TEXT
  header[2] = id >> 8;
  header[3] = id & 0xFF;
  header[4] = buf.size() >> 24;
  header[5] = (buf.size() >> 16) & 0xFF;
  header[6] = (buf.size() >> 8) & 0xFF;
  header[7] = buf.size() & 0xFF;

  Broadcast_(header);
  Broadcast_(buf);
}

void RegisterName_(uint16_t id, std::string name) {
  std::vector<uint8_t> header(8);
  header[0] = 0x00;
  header[1] = 0x01;  // Link Name
  header[2] = id >> 8;
  header[3] = id & 0xFF;
  header[4] = name.length() >> 24;
  header[5] = (name.length() >> 16) & 0xFF;
  header[6] = (name.length() >> 8) & 0xFF;
  header[7] = name.length() & 0xFF;

  Broadcast_(header);
  Broadcast_(std::vector<uint8_t>(name.begin(), name.end()));
}

}  // namespace

UartStreaming::UartStreaming(std::string name) {
  static int last_id = 0;
  this->id = last_id++;
  RegisterName_(id, name);
}

void UartStreaming::HandleUart(void *arg) {
  UartStreaming *self = static_cast<UartStreaming *>(arg);

  auto length = self->uart->GetRXBufferDataLength() + 20;
  std::vector<uint8_t> buf(length);
  self->uart->Recv(buf, 500 / portTICK_PERIOD_MS);

  BroadcastLog_(self->id, buf);
}