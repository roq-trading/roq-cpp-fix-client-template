/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/samples/fix_client/crypto.hpp"

#include <algorithm>
#include <random>
#include <string>

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

#include "roq/utils/codec/base64.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

// === CONSTANTS ===

namespace {
auto const NONCE_LENGTH = size_t{32};
auto const NONCE_CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789"sv;
std::random_device NONCE_GENERATOR;
std::uniform_int_distribution<size_t> NONCE_DISTRIBUTION(0, std::size(NONCE_CHARSET) - 1);
}  // namespace

// === HELPERS ===

namespace {
auto create_nonce() {
  std::string result;
  result.resize(NONCE_LENGTH);
  std::generate(
      std::begin(result), std::end(result), []() { return NONCE_CHARSET[NONCE_DISTRIBUTION(NONCE_GENERATOR)]; });
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(Settings const &settings) : settings_{settings}, mac_{settings_.fix.password} {
}

codec::fix::Logon Crypto::create_logon([[maybe_unused]] std::chrono::nanoseconds sending_time_utc) {
  auto heart_bt_int = static_cast<decltype(codec::fix::Logon::heart_bt_int)>(
      std::chrono::duration_cast<std::chrono::seconds>(settings_.fix.ping_freq).count());
  if (std::empty(settings_.fix.auth_method) ||
      utils::case_insensitive_compare(settings_.fix.auth_method, "simple"sv) == 0) {
    return {
        .encrypt_method = roq::fix::EncryptMethod::NONE,
        .heart_bt_int = heart_bt_int,
        .raw_data_length = {},
        .raw_data = {},
        .reset_seq_num_flag = true,
        .next_expected_msg_seq_num = 1,  // note!
        .username = settings_.fix.username,
        .password = settings_.fix.password,
    };
  } else if (utils::case_insensitive_compare(settings_.fix.auth_method, "hmac_sha256"sv) == 0) {
    nonce_ = create_nonce();
    assert(std::size(nonce_) == NONCE_LENGTH);
    hash_.clear();
    hash_.update(nonce_);
    auto digest = hash_.final(digest_);
    buffer_.clear();
    utils::codec::Base64::encode(buffer_, digest, false, false);
    return {
        .encrypt_method = roq::fix::EncryptMethod::NONE,
        .heart_bt_int = heart_bt_int,
        .raw_data_length = {},
        .raw_data = nonce_,
        .reset_seq_num_flag = true,
        .next_expected_msg_seq_num = 1,  // note!
        .username = settings_.fix.username,
        .password = buffer_,
    };
  } else if (utils::case_insensitive_compare(settings_.fix.auth_method, "hmac_sha256_ts"sv) == 0) {
    log::fatal(R"(Unexpected: auth_method="{}")"sv, settings_.fix.auth_method);
  } else {
    log::fatal(R"(Unexpected: auth_method="{}")"sv, settings_.fix.auth_method);
  }
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
