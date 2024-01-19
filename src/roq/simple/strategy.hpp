/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/simple/settings.hpp"

namespace roq {
namespace simple {

struct Strategy final {
  Strategy(Settings const &);

  Strategy(Strategy &&) = default;
  Strategy(Strategy const &) = delete;

  void dispatch();
};

}  // namespace simple
}  // namespace roq
