/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Strategy final {
  Strategy(Settings const &);

  Strategy(Strategy &&) = default;
  Strategy(Strategy const &) = delete;

  void dispatch();
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
