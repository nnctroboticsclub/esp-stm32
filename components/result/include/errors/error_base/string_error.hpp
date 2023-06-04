#pragma once

#include <string>

#include "errorbase.hpp"

class StringError : public ErrorBase {
  std::string err;

 public:
  StringError(std::string err);

  bool IsError() override;
  const char* what() override;
};