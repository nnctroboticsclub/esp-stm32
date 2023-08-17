#pragma once

#include <esp_http_server.h>
#include <stm32bl.hpp>
#include <memory>

using namespace connection::application::stm32bl;

class DebuggerHTTPServer {
  static constexpr const char *TAG = "Debug HTTPd";
  httpd_handle_t httpd = nullptr;
  std::unique_ptr<STM32BootLoader> bl;

  static esp_err_t BL_Boot(httpd_req_t *req) {
    auto &obj = *(DebuggerHTTPServer *)req->user_ctx;
    auto &bl = obj.bl;

    bl->Connect();

    return ESP_OK;
  }
  static esp_err_t BL_Upload(httpd_req_t *req) {
    auto &obj = *(DebuggerHTTPServer *)req->user_ctx;
    auto &bl = obj.bl;

    auto size = req->content_len;

    // Erasing...
    bl->Erase(0x0800'0000, size);

    // Receive & Write Loop
    size_t read = 0;
    std::vector<uint8_t> buf(2048);
    while (read < size) {
      auto data_size_to_read = std::min(int(size - read), 2048);
      buf.resize(data_size_to_read);

      ESP_ERROR_CHECK(
          httpd_req_recv(req, (char *)buf.data(), data_size_to_read));

      bl->WriteMemory(0x0800'0000 + read, buf);
    }
    return ESP_OK;
  }
  static esp_err_t BL_Go(httpd_req_t *req) {
    auto &obj = *(DebuggerHTTPServer *)req->user_ctx;
    auto &bl = obj.bl;

    bl->Go(0x0800'0000);
    return ESP_OK;
  }

 public:
  DebuggerHTTPServer(std::unique_ptr<STM32BootLoader> bl) : bl(std::move(bl)){};

  void Listen(int port = 80) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.global_user_ctx = this;

    ESP_ERROR_CHECK(httpd_start(&httpd, &config));

    const httpd_uri_t handlers[] = {{.uri = "/api/bootloader/boot",
                                     .method = HTTP_POST,
                                     .handler = DebuggerHTTPServer::BL_Boot,
                                     .user_ctx = (void *)&this->bl},
                                    {.uri = "/api/bootloader/go",
                                     .method = HTTP_POST,
                                     .handler = DebuggerHTTPServer::BL_Boot,
                                     .user_ctx = (void *)&this->bl},
                                    {
                                        .uri = "/api/bootloader/upload",
                                        .method = HTTP_POST,
                                        .handler = DebuggerHTTPServer::BL_Boot,
                                        .user_ctx = (void *)&this->bl
                                        // upload : erase + write
                                    }};

    for (auto &&handler : handlers) {
      ESP_LOGI(TAG, "Registering Endpoint: %s", handler.uri);
      ESP_ERROR_CHECK(httpd_register_uri_handler(this->httpd, &handler));
    }
  }
};