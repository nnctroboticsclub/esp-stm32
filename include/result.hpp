#pragma once

#include <optional>
#include <esp_err.h>
#include <string>

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

template <typename T>
class Result {
  std::optional<T> value;
  esp_err_t error;

  Result(std::optional<T> value, esp_err_t error)
      : value(value), error(error) {}

 public:
  // Result(T value) : value(value), error(ESP_OK) {}
  Result(esp_err_t error) : value(std::nullopt), error(error) {}

  inline bool IsOk() { return error == ESP_OK; }
  inline bool IsErr() { return error != ESP_OK; }

  inline T& Value() { return value.value(); }
  inline esp_err_t Error() { return error; }

  inline T& operator*() { return value.value(); }
  inline T* operator->() { return &value.value(); }

  inline static Result<T> Ok(T value) { return Result<T>(value, ESP_OK); }
  inline static Result<T> Err(esp_err_t error) {
    return Result<T>(std::nullopt, error);
  }
};

template <>
class Result<void> {
  esp_err_t error;

 public:
  Result(esp_err_t error) : error(error) {}
  Result() : error(ESP_OK) {}

  inline bool IsOk() { return error == ESP_OK; }
  inline bool IsErr() { return error != ESP_OK; }

  inline esp_err_t Error() { return error; }

  inline static Result<void> Ok() { return Result<void>(ESP_OK); }
};
using TaskResult = Result<void>;
