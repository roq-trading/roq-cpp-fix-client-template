/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/simple/application.hpp"

#include <cassert>
#include <vector>

#include "roq/client.hpp"

#include "roq/simple/settings.hpp"

using namespace std::chrono_literals;
using namespace std::literals;

namespace roq {
namespace simple {

// === IMPLEMENTATION ===

int Application::main(roq::args::Parser const &args) {
  auto params = args.params();
  if (std::empty(params))
    roq::log::fatal("Unexpected"sv);
  Settings settings{args};
  Strategy{settings}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace simple
}  // namespace roq
