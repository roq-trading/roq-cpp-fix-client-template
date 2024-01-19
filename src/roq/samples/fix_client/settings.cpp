/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

Settings::Settings(roq::args::Parser const &args)
    : flags::Flags{flags::Flags::create()}, fix{flags::FIX::create()}, test{flags::Test::create()} {
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
