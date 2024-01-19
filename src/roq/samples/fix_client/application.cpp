/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/samples/fix_client/application.hpp"

#include <cassert>
#include <vector>

#include "roq/io/engine/context_factory.hpp"

#include "roq/samples/fix_client/settings.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

// === IMPLEMENTATION ===

int Application::main(roq::args::Parser const &args) {
  auto params = args.params();
  if (std::empty(params))
    roq::log::fatal("Unexpected"sv);
  Settings settings{args};
  auto context = io::engine::ContextFactory::create_libevent();
  Strategy{settings, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
