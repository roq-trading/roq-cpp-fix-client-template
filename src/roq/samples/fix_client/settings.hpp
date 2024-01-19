/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/args/parser.hpp"

#include "roq/samples/fix_client/flags/fix.hpp"
#include "roq/samples/fix_client/flags/flags.hpp"
#include "roq/samples/fix_client/flags/test.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Settings final : public flags::Flags {
  explicit Settings(roq::args::Parser const &);

  flags::FIX const fix;
  flags::Test const test;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
