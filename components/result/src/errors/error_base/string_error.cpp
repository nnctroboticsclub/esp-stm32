#include <errors/error_base/string_error.hpp>

#include <esp_log.h>
#include <string.h>

StringError::StringError(std::string err) : err(err) {}

bool StringError::IsError() { return true; }
const char *StringError::what() { return this->err.c_str(); }