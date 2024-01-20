/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

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

template <>
struct fmt::formatter<roq::samples::fix_client::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::samples::fix_client::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(fix={}, )"
        R"(test={}, )"
        R"({})"
        R"(}})"sv,
        value.fix,
        value.test,
        static_cast<roq::samples::fix_client::flags::Flags const &>(value));
  }
};
