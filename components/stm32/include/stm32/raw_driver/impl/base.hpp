#pragma once

#include <cinttypes>
#include <type_traits>

#include "../types/inbounddata.hpp"
#include "../types/outbounddata.hpp"

namespace stm32::raw_driver {

InboundData _dummy;

template <typename T>
concept RawDriverConcept =
    requires(T t) {  // NOLINT -- cv specifier is not needed.
      { t.ACK() } -> std::same_as<void>;
      { t.Send(OutboundData{}) } -> std::same_as<void>;

      { t.Recv(_dummy) } -> std::same_as<void>;
      { t.CommandHeader((uint8_t)0) } -> std::same_as<void>;

      { t.Sync() } -> std::same_as<void>;
    };  // NOLINT -- require's semicolon is needed

}  // namespace stm32::raw_driver
