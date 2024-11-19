/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>

#include "roq/utils/metrics/writer.hpp"

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

struct Shared final {
  explicit Shared(Settings const &);

  Shared(Shared const &) = delete;

  Settings const &settings;
  std::string encode_buffer;
  std::unique_ptr<utils::metrics::Writer> metrics_writer;
};

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
