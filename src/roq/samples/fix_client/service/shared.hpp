/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

struct Shared final {
  explicit Shared(Settings const &);

  Shared(Shared const &) = delete;

  Settings const &settings;
  std::string encode_buffer;
};

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
