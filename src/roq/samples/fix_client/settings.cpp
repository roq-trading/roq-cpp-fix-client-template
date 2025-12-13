/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/samples/fix_client/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

Settings::Settings(roq::args::Parser const &args)
    : fix::client::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_VERSION}, flags::Flags{flags::Flags::create()}, service{flags::Service::create()},
      test{flags::Test::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
