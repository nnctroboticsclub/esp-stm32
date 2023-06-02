#pragma once

#include <optional>
#include <esp_err.h>
#include <string>
#include <memory>
#include <string.h>

// Macros for unwrapping result
#define CAT_IMPL(s1, s2) s1##s2
#define CAT(s1, s2) CAT_IMPL(s1, s2)

// Run a task and forwarding the error code
#define _RUN_TASK_V(result, res) \
  auto res = result;             \
  if (res.IsErr()) {             \
    return res.Error();          \
  }

// Run a task and forwarding the error code, and assign the result to a variable
#define _RUN_TASK(result, var, res) \
  _RUN_TASK_V(result, res)          \
  auto var = res.Value();

// Run a task and forwarding the error code, and assign the result to a variable
#define _RUN_TASK_TO(result, var, res) \
  _RUN_TASK_V(result, res)             \
  var = res.Value();

// Get Unique variable name
#define UNIQUE_NAME(base) CAT(base, CAT(__LINE__, __COUNTER__))

// short hands for some common cases
#define RUN_TASK_V(result) _RUN_TASK_V(result, UNIQUE_NAME(res))
#define RUN_TASK(result, var) _RUN_TASK(result, var, UNIQUE_NAME(res))
#define RUN_TASK_TO(result, var) _RUN_TASK_TO(result, var, UNIQUE_NAME(res))

class ErrorBase {
 public:
  virtual const char* what() = 0;

  virtual bool IsError() { return true; }
};

class EspError : public ErrorBase {
  esp_err_t err;

 public:
  EspError(esp_err_t err) : err(err) {}

  const char* what() override { return esp_err_to_name(err); }

  bool IsError() override { return err != ESP_OK; }
};

class ErrnoError : public ErrorBase {
  int err;

 public:
  ErrnoError(int err) : err(err) {}

  const char* what() override { return strerror(err); }

  bool IsError() override { return err != 0; }
};

class StringError : public ErrorBase {
  std::string err;

 public:
  StringError(std::string err) : err(err) {}

  const char* what() override { return err.c_str(); }
};

class NoError : public ErrorBase {
 private:
  static NoError instance;

  NoError() {}

 public:
  const char* what() override { return "No Error"; }

  bool IsError() override { return false; }

  static NoError& Get() { return instance; }
};

class ErrorType {
  std::shared_ptr<ErrorBase> error;

 public:
  ErrorType(ErrorBase& error) : error(&error) {}
  ErrorType(ErrorBase* error) : error(error) {}
  ErrorType(esp_err_t err) : error(std::make_shared<EspError>(err)) {}
  ErrorType(std::string err) : error(std::make_shared<StringError>(err)) {}
  ErrorType() : error(std::make_shared<NoError>(NoError::Get())) {}

  bool IsError() { return this->error.get()->IsError(); }
  const char* what() { return this->error.get()->what(); }
};

template <typename T>
class Result {
  std::optional<T> value;
  ErrorType error;

 public:
  Result(ErrorType& error, std::optional<T> value = std::nullopt)
      : value(value), error(error) {}

  Result(ErrorBase& error, std::optional<T> value = std::nullopt)
      : value(value), error(error) {}
  Result(ErrorBase* error, std::optional<T> value = std::nullopt)
      : value(value), error(error) {}

  Result(esp_err_t error, std::optional<T> value = std::nullopt)
      : value(value), error(error) {}

  Result(std::string error, std::optional<T> value = std::nullopt)
      : value(value), error(error) {}

  static Result<T> Ok(T value) { return Result<T>(NoError::Get(), value); }
  Result(std::optional<T> value) : value(value), error(NoError::Get()) {}

  inline bool IsOk() { return value.has_value(); }
  inline bool IsErr() { return this->error.IsError(); }

  inline T& Value() { return value.value(); }
  inline ErrorType& Error() { return this->error; }

  inline T& operator*() { return value.value(); }
  inline T* operator->() { return &value.value(); }
};

template <>
class Result<void> {
  ErrorType error;

 public:
  Result(ErrorBase& error) : error(error) {}
  Result(ErrorType& error) : error(error) {}
  Result(esp_err_t error) : error(error) {}
  Result(std::string error) : error(error) {}
  Result() : error(NoError::Get()) {}

  inline bool IsOk() { return !this->error.IsError(); }
  inline bool IsErr() { return this->error.IsError(); }

  inline ErrorType& Error() { return this->error; }

  inline static Result<void> Ok() { return Result<void>(); }
};
using TaskResult = Result<void>;
