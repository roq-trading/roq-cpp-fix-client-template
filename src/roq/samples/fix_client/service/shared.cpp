/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/fix_client/service/shared.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

// === HELPERS ===

namespace {
auto create_metrics_writer() {
  return utils::metrics::Writer::create();
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(Settings const &settings) : settings{settings}, metrics_writer{create_metrics_writer()} {
}

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
