#include <errors/error_type.hpp>

#include <esp_log.h>

// #include <errors/error_base/errno_error.hpp>
// #include <errors/error_base/esp_error.hpp>
// #include <errors/error_base/noerror.hpp>
// #include <errors/error_base/string_error.hpp>

ErrorType::ErrorType(ErrorBase& error) : error(&error) {}
ErrorType::ErrorType(ErrorBase* error) : error(error) {}
ErrorType::ErrorType(esp_err_t err) : error(std::make_shared<EspError>(err)) {}
ErrorType::ErrorType(std::string err)
    : error(std::make_shared<StringError>(err)) {}
ErrorType::ErrorType() : error(std::make_shared<NoError>(*(new NoError()))) {}

bool ErrorType::IsError() { return this->error.get()->IsError(); }
const char* ErrorType::what() { return this->error.get()->what(); }