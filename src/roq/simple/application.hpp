/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/service.hpp"

// note! the following are your implementations

#include "roq/simple/settings.hpp"
#include "roq/simple/strategy.hpp"

namespace roq {
namespace simple {

struct Application final : public roq::Service {
  using Service::Service;  // inherit constructors

 protected:
  int main(roq::args::Parser const &) override;

 private:
  using value_type = Strategy;  // note!
};

}  // namespace simple
}  // namespace roq
