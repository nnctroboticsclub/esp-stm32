#pragma once

#include <esp_err.h>

#include "errorbase.hpp"

class EspError : public ErrorBase {
  esp_err_t err;

 public:
  EspError(esp_err_t err);

  const char* what() override;

  bool IsError() override;
};