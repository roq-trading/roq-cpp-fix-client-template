/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/utils/hash/sha256.hpp"

#include "roq/utils/mac/hmac.hpp"

#include "roq/codec/fix/logon.hpp"

#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace session {

struct Crypto final {
  explicit Crypto(Settings const &);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  codec::fix::Logon create_logon(std::chrono::nanoseconds sending_time_utc);

  enum class Method {
    UNDEFINED,
    HMAC_SHA256,
    HMAC_SHA256_TS,
  };

 private:
  using Hash = utils::hash::SHA256;
  using MAC = utils::mac::HMAC<utils::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  Settings const &settings_;
  Method const method_;
  Hash hash_;
  MAC mac_;
  Digest digest_;
  std::string raw_data_;
  std::string signature_;
};

}  // namespace session
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
