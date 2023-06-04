#include <errors/error_base/esp_error.hpp>

#include <esp_log.h>

EspError::EspError(esp_err_t err) : err(err) {}

const char *EspError::what() { return esp_err_to_name(this->err); }

bool EspError::IsError() { return this->err != ESP_OK; }