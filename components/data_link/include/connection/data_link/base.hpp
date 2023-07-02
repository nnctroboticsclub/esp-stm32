#pragma once

#include <vector>
#include <stdexcept>

#include <freertos/FreeRTOS.h>

namespace connection::data_link {

class ConnectionClosedError : public std::runtime_error {
 public:
  ConnectionClosedError() : std::runtime_error("Connection closed") {}
};

class RecvAndSend {
 public:
  virtual size_t Send(std::vector<uint8_t> &buf) = 0;
  virtual size_t Recv(std::vector<uint8_t> &buf,
                      TickType_t timeout = 1000 / portTICK_PERIOD_MS) = 0;

  virtual ~RecvAndSend();

  template <int N>
  size_t Send(const char (&buf)[N]) {
    std::vector<uint8_t> v{buf, buf + N};
    return this->Send(v);
  }

  char RecvChar(TickType_t timeout = 1000 / portTICK_PERIOD_MS) {
    std::vector<uint8_t> c{0};
    this->Recv(c, timeout);
    return c[0];
  }

  void SendChar(char ch) {
    std::vector<uint8_t> c;
    c[0] = ch;
    this->Send(c);
  }

  void RecvExactly(std::vector<uint8_t> &buf,
                   TickType_t timeout = 1000 / portTICK_PERIOD_MS) {
    size_t read = 0;
    auto size = buf.size();
    while (read < size) {
      std::vector<uint8_t> sub_vector{buf.begin() + read, buf.end()};
      read += this->Recv(sub_vector, timeout);
    }
  }

  std::vector<uint8_t> RecvUntil(char delim,
                                 TickType_t timeout = portMAX_DELAY) {
    std::vector<uint8_t> ret;
    while (true) {
      auto c = this->RecvChar(timeout);
      if (c == delim) {
        return ret;
      }
      ret.emplace_back(c);
    }
    return ret;
  }

  void SendU16(uint16_t value) {
    std::vector<uint8_t> buf;
    buf[0] = (value >> 8) & 0xff;
    buf[1] = value & 0xff;

    this->Send(buf);
  }

  void SendU32(uint32_t value) {
    std::vector<uint8_t> buf;
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;

    this->Send(buf);
  }

  uint32_t RecvU32() {
    std::vector<uint8_t> buf(4);
    this->RecvExactly(buf);
    return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
  }
};
}  // namespace connection::data_link
