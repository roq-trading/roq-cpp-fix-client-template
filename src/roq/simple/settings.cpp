/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/simple/settings.hpp"

namespace roq {
namespace simple {

Settings::Settings(roq::args::Parser const &args)
    : flags::Flags{flags::Flags::create()}, fix{flags::FIX::create()}, test{flags::Test::create()} {
}

}  // namespace simple
}  // namespace roq
