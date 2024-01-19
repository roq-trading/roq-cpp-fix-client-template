/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/io/context.hpp"

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Strategy final {
  Strategy(Settings const &, io::Context &);

  Strategy(Strategy &&) = default;
  Strategy(Strategy const &) = delete;

  void dispatch();

 private:
  Settings const &settings_;
  io::Context &context_;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
