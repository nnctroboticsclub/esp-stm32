#pragma once

#include "errorbase.hpp"

class ErrnoError : public ErrorBase {
  int err;

 public:
  ErrnoError(int err);

  const char* what() override;

  bool IsError() override;
};