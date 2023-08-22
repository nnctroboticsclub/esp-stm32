#pragma once

#include <string>
#include <memory>
#include <vector>
#include <spi_host_cxx.hpp>

#include "../libs/nvs_proxy.hpp"

namespace profile {
class SPIBus : public nvs::Namespace {
  std::shared_ptr<idf::SPIMaster> dev;

  nvs::Proxy<uint8_t> spi_port;
  nvs::Proxy<gpio_num_t> miso;
  nvs::Proxy<gpio_num_t> mosi;
  nvs::Proxy<gpio_num_t> sclk;

 public:
  explicit SPIBus(std::string const& ns);

  std::shared_ptr<idf::SPIMaster> GetDevice() const;
};
}  // namespace profile
