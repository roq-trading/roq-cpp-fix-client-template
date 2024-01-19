/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/service.hpp"

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Application final : public roq::Service {
  using Service::Service;

 protected:
  int main(roq::args::Parser const &) override;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
