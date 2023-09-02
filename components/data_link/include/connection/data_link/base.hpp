#pragma once

#include <vector>
#include <stdexcept>

#include <freertos/FreeRTOS.h>

namespace connection::data_link {

class ConnectionClosedError : public std::runtime_error {
 public:
  ConnectionClosedError() : std::runtime_error("Connection closed") {}
};

class Receivable {
  bool trace_enabled = false;

 public:
  inline bool IsTraceEnabled() const { return this->trace_enabled; }
  inline void SetTraceEnabled(bool enabled) { this->trace_enabled = enabled; }

  virtual size_t Recv(std::vector<uint8_t> &buf,
                      TickType_t timeout = 1000 / portTICK_PERIOD_MS) = 0;

  virtual ~Receivable();

  char RecvChar(TickType_t timeout = 1000 / portTICK_PERIOD_MS) {
    std::vector<uint8_t> c{0};
    this->Recv(c, timeout);
    return c[0];
  }

  void RecvExactly(std::vector<uint8_t> &buf,
                   TickType_t timeout = 1000 / portTICK_PERIOD_MS) {
    size_t read = 0;
    auto size = buf.size();
    while (read < size) {
      std::vector<uint8_t> sub_vector{buf.begin() + read, buf.end()};
      auto chunk_size = this->Recv(sub_vector, timeout);

      std::copy(sub_vector.begin(), sub_vector.end(), buf.begin() + read);

      read += chunk_size;
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

  uint32_t RecvU32() {
    std::vector<uint8_t> buf(4);
    this->RecvExactly(buf);
    return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
  }
};

class Sendable {
  bool trace_enabled = false;

 public:
  inline bool IsTraceEnabled() const { return this->trace_enabled; }
  inline void SetTraceEnabled(bool enabled) { this->trace_enabled = enabled; }

  virtual size_t Send(const std::vector<uint8_t> &buf) = 0;

  virtual ~Sendable();

  template <int N>
  size_t Send(const char (&buf)[N]) {
    std::vector<uint8_t> v{buf, buf + N};
    return this->Send(v);
  }

  void SendChar(const char ch) {
    std::vector<uint8_t> c(1);
    c[0] = ch;
    this->Send(c);
  }

  void SendU16(const uint16_t value) {
    std::vector<uint8_t> buf(2);
    buf[0] = (value >> 8) & 0xff;
    buf[1] = value & 0xff;

    this->Send(buf);
  }

  void SendU32(const uint32_t value) {
    std::vector<uint8_t> buf(4);
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;

    this->Send(buf);
  }
};

class RecvAndSend : public Receivable, public Sendable {
 public:
  ~RecvAndSend();
  inline bool IsTraceEnabled() const {
    return Receivable::IsTraceEnabled() || Sendable::IsTraceEnabled();
  }
  inline void SetTraceEnabled(bool enabled) {
    Receivable::SetTraceEnabled(enabled);
    Sendable::SetTraceEnabled(enabled);
  }
};
}  // namespace connection::data_link
