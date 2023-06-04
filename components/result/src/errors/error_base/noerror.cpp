#include <errors/error_base/noerror.hpp>

#include <esp_log.h>

const char *NoError::what() { return "No Error"; }

bool NoError::IsError() { return false; }