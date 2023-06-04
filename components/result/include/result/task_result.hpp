#include "result.hpp"

#include <esp_err.h>

template <>
class Result<void> {
  ErrorType error;

 public:
  Result(ErrorBase& error) : error(error) {}
  Result(ErrorType& error) : error(error) {}
  Result(esp_err_t error) : error(error) {}
  Result(std::string error) : error(error) {}
  Result() : Result(*(new NoError())) {}

  inline bool IsOk() { return !this->error.IsError(); }
  inline bool IsErr() { return this->error.IsError(); }

  inline ErrorType& Error() { return this->error; }

  inline static Result<void> Ok() { return Result<void>(); }
};
using TaskResult = Result<void>;