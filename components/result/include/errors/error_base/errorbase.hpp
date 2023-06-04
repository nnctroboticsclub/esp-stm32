#pragma once

class ErrorBase {
 public:
  virtual ~ErrorBase() = default;

  virtual const char* what() = 0;

  virtual bool IsError() { return true; }
};