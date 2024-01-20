/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/utils/hash/sha256.hpp"

#include "roq/utils/mac/hmac.hpp"

#include "roq/codec/fix/logon.hpp"

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Crypto final {
  using Hash = utils::hash::SHA256;
  using MAC = utils::mac::HMAC<utils::hash::SHA256>;

  explicit Crypto(Settings const &);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  codec::fix::Logon create_logon(std::chrono::nanoseconds sending_time_utc);

 private:
  Settings const &settings_;
  std::array<std::byte, MAC::DIGEST_LENGTH> digest_;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
