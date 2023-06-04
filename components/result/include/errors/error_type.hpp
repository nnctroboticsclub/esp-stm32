#pragma once

#include <memory>
#include <string>

#include "error_base/errorbase.hpp"
#include <errors/error_base/errno_error.hpp>
#include <errors/error_base/esp_error.hpp>
#include <errors/error_base/noerror.hpp>
#include <errors/error_base/string_error.hpp>

class ErrorType {
  std::shared_ptr<ErrorBase> error;

 public:
  ErrorType(ErrorBase& error);
  ErrorType(ErrorBase* error);
  ErrorType(esp_err_t err);
  ErrorType(std::string err);
  ErrorType();

  bool IsError();
  const char* what();
};