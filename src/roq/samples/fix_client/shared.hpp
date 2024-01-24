/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/metrics/writer.hpp"

#include "roq/utils/metrics/histogram.hpp"

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Shared {
  explicit Shared(Settings const &settings);

  Shared(Shared const &) = delete;

  void operator()(metrics::Writer &);

  struct {
    utils::metrics::external_latency_t internal, external;
  } request_latency;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
