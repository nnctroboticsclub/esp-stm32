#pragma once

#include <esp_http_server.h>
#include <memory>
#include <vector>

#include "config/config.hpp"

class DebuggerHTTPServer {
  static constexpr const char *TAG = "Debug HTTPd";
  httpd_handle_t httpd = nullptr;
  std::shared_ptr<stm32::STM32> stm32;
  std::optional<stm32::session::BootLoaderSession> stm32_bl;

  static esp_err_t BL_Boot(httpd_req_t *req) {
    auto &obj = *static_cast<DebuggerHTTPServer *>(req->user_ctx);
    if (!obj.stm32_bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "Bootloader is already launched");
      return ESP_FAIL;
    }

    auto bl = obj.stm32->EnterBootloader();
    if (!bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "Failed to enter Bootloader");
      return ESP_FAIL;
    }

    obj.stm32_bl = bl;

    return ESP_OK;
  }
  static esp_err_t BL_Upload(httpd_req_t *req) {
    auto &obj = *static_cast<DebuggerHTTPServer *>(req->user_ctx);
    if (!obj.stm32_bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "No Bootloader is launched");
      return ESP_FAIL;
    }
    auto bl = *obj.stm32_bl;

    auto size = req->content_len;

    // Erasing...
    bl.Erase(stm32::driver::ErasePages(0x0800'0000, size));

    // Receive & Write Loop
    size_t read = 0;
    std::vector<uint8_t> buf(2048);
    while (read < size) {
      auto data_size_to_read = std::min(int(size - read), 2048);
      buf.resize(data_size_to_read);

      ESP_ERROR_CHECK(httpd_req_recv(req, reinterpret_cast<char *>(buf.data()),
                                     data_size_to_read));

      bl.WriteMemory(0x0800'0000 + read, buf);
    }
    return ESP_OK;
  }
  static esp_err_t BL_Go(httpd_req_t *req) {
    auto &obj = *static_cast<DebuggerHTTPServer *>(req->user_ctx);
    if (!obj.stm32_bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "No Bootloader is launched");
      return ESP_FAIL;
    }
    auto bl = *obj.stm32_bl;

    bl.Go(0x0800'0000);
    obj.stm32_bl.reset();
    return ESP_OK;
  }

 public:
  explicit DebuggerHTTPServer(std::shared_ptr<stm32::STM32> stm32)
      : stm32(stm32) {}

  void Listen(int port = 80) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.global_user_ctx = this;

    ESP_ERROR_CHECK(httpd_start(&httpd, &config));

    const httpd_uri_t handlers[] = {{.uri = "/api/bootloader/boot",
                                     .method = HTTP_POST,
                                     .handler = DebuggerHTTPServer::BL_Boot,
                                     .user_ctx = static_cast<void *>(this)},
                                    {.uri = "/api/bootloader/go",
                                     .method = HTTP_POST,
                                     .handler = DebuggerHTTPServer::BL_Boot,
                                     .user_ctx = static_cast<void *>(this)},
                                    {
                                        .uri = "/api/bootloader/upload",
                                        .method = HTTP_POST,
                                        .handler = DebuggerHTTPServer::BL_Boot,
                                        .user_ctx = static_cast<void *>(this)
                                        // upload : erase + write
                                    }};

    for (auto &&handler : handlers) {
      ESP_LOGI(TAG, "Registering Endpoint: %s", handler.uri);
      ESP_ERROR_CHECK(httpd_register_uri_handler(this->httpd, &handler));
    }
  }
};