/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/service.hpp"

#include "roq/samples/fix_client/settings.hpp"
#include "roq/samples/fix_client/strategy.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Application final : public roq::Service {
  using Service::Service;  // inherit constructors

 protected:
  int main(roq::args::Parser const &) override;

 private:
  using value_type = Strategy;  // note!
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
