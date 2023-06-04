#pragma once

#include <optional>

#include <esp_log.h>

#include "../errors/error_type.hpp"
#include "../errors/error_base/noerror.hpp"

template <typename T>
class Result {
  std::optional<T> value;
  ErrorType error;

 public:
  Result(ErrorType& error_, std::optional<T> value = std::nullopt)
      : value(value), error(error_) {}

  Result(ErrorBase& error_, std::optional<T> value = std::nullopt)
      : value(value), error(error_) {}

  Result(ErrorBase* error_, std::optional<T> value = std::nullopt)
      : value(value), error(error_) {}
  Result(esp_err_t error_, std::optional<T> value = std::nullopt)
      : value(value), error(error_) {}
  Result(std::string error_, std::optional<T> value = std::nullopt)
      : value(value), error(error_) {}

  static Result<T> Ok(T value) {
    auto v = std::optional<T>(value);
    return Result<T>(new NoError(), v);
  }
  Result(std::optional<T> value) : value(value), error(new NoError()) {}

  inline bool IsOk() { return value.has_value(); }
  inline bool IsErr() {
    auto& err = this->error;
    auto ret = err.IsError();
    return ret;
  }

  inline T& Value() { return value.value(); }
  inline ErrorType& Error() { return this->error; }

  inline T& operator*() { return value.value(); }
  inline T* operator->() { return &value.value(); }
};