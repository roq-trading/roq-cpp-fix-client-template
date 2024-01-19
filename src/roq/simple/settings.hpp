/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/args/parser.hpp"

#include "roq/simple/flags/fix.hpp"
#include "roq/simple/flags/flags.hpp"
#include "roq/simple/flags/test.hpp"

namespace roq {
namespace simple {

struct Settings final : public flags::Flags {
  explicit Settings(roq::args::Parser const &);

  flags::FIX const fix;
  flags::Test const test;
};

}  // namespace simple
}  // namespace roq
