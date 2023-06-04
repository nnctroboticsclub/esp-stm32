#pragma once

#include "errorbase.hpp"

class NoError : public ErrorBase {
 public:
  NoError() = default;

  const char* what() override;

  bool IsError() override;
};