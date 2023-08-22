#include "spibus.hpp"

namespace profile {

SPIBus::SPIBus(std::string const& ns)
    : nvs::Namespace(ns),
      spi_port(this, "spi_port"),
      miso(this, "miso"),
      mosi(this, "mosi"),
      sclk(this, "sclk") {
  if ((gpio_num_t)this->sclk != 0) {
    this->dev = std::make_shared<idf::SPIMaster>(this->spi_port, this->miso,
                                                 this->mosi, this->sclk);
  }
}

std::shared_ptr<idf::SPIMaster> SPIBus::GetDevice() const {
  if (this->dev == nullptr) {
    throw std::runtime_error("SPI bus not initialised");
  }
  return dev;
}

}  // namespace profile