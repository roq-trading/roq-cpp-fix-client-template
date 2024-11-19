/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/fix_client/shared.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

// === HELPERS ===

namespace {
auto create_histogram(auto &settings, auto const &function) {
  auto labels = fmt::format(R"(source="{}", function="{}")"sv, settings.name, function);
  return utils::metrics::external_latency_t{labels};
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(Settings const &settings)
    : request_latency{
          .internal = create_histogram(settings, "internal"sv),
          .external = create_histogram(settings, "external"sv),
      } {
}

void Shared::operator()(metrics::Writer &writer) {
  writer  //
      .write(request_latency.internal, metrics::Type::REQUEST_LATENCY)
      .write(request_latency.external, metrics::Type::REQUEST_LATENCY);
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
