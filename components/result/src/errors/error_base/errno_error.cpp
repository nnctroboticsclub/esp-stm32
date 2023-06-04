#include <errors/error_base/errno_error.hpp>

#include <esp_log.h>
#include <string.h>

ErrnoError::ErrnoError(int err) : err(err) {}

bool ErrnoError::IsError() { return this->err != 0; }
const char *ErrnoError::what() { return strerror(this->err); }