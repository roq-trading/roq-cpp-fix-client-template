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
auto const TIMESTAMP_LENGTH = size_t{13};  // note! milliseconds
auto const NONCE_LENGTH = size_t{32};
auto const NONCE_CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789"sv;
std::random_device NONCE_GENERATOR;
std::uniform_int_distribution<size_t> NONCE_DISTRIBUTION(0, std::size(NONCE_CHARSET) - 1);
}  // namespace

// === HELPERS ===

namespace {
auto parse_method(auto &method) {
  if (std::empty(method))
    return Crypto::Method::UNDEFINED;
  if (method == "hmac_sha256"sv)
    return Crypto::Method::HMAC_SHA256;
  if (method == "hmac_sha256_ts"sv)
    return Crypto::Method::HMAC_SHA256_TS;
  log::fatal(R"(Unexpected: auth_method="{}")"sv, method);
}

auto create_nonce() {
  std::string result;
  result.resize(NONCE_LENGTH);
  std::generate(
      std::begin(result), std::end(result), []() { return NONCE_CHARSET[NONCE_DISTRIBUTION(NONCE_GENERATOR)]; });
  return result;
}

auto create_nonce(auto sending_time_utc) {
  return fmt::format(
      "{}.{}"sv, std::chrono::duration_cast<std::chrono::milliseconds>(sending_time_utc).count(), create_nonce());
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(Settings const &settings)
    : settings_{settings}, method_{parse_method(settings.fix.auth_method)}, mac_{settings_.fix.password} {
}

codec::fix::Logon Crypto::create_logon([[maybe_unused]] std::chrono::nanoseconds sending_time_utc) {
  auto heart_bt_int = static_cast<decltype(codec::fix::Logon::heart_bt_int)>(
      std::chrono::duration_cast<std::chrono::seconds>(settings_.fix.ping_freq).count());
  switch (method_) {
    using enum Method;
    case UNDEFINED:
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
    case HMAC_SHA256: {
      mac_.clear();
      raw_data_ = create_nonce();
      assert(std::size(raw_data_) == NONCE_LENGTH);
      mac_.update(raw_data_);
      auto digest = mac_.final(digest_);
      signature_.clear();
      utils::codec::Base64::encode(signature_, digest, false, false);
      return {
          .encrypt_method = roq::fix::EncryptMethod::NONE,
          .heart_bt_int = heart_bt_int,
          .raw_data_length = {},
          .raw_data = raw_data_,
          .reset_seq_num_flag = true,
          .next_expected_msg_seq_num = 1,  // note!
          .username = settings_.fix.username,
          .password = signature_,
      };
    }
    case HMAC_SHA256_TS: {
      mac_.clear();
      raw_data_ = create_nonce(sending_time_utc);
      assert(std::size(raw_data_) == (TIMESTAMP_LENGTH + 1 + NONCE_LENGTH));
      mac_.update(raw_data_);
      auto digest = mac_.final(digest_);
      signature_.clear();
      utils::codec::Base64::encode(signature_, digest, false, false);
      return {
          .encrypt_method = roq::fix::EncryptMethod::NONE,
          .heart_bt_int = heart_bt_int,
          .raw_data_length = {},
          .raw_data = raw_data_,
          .reset_seq_num_flag = true,
          .next_expected_msg_seq_num = 1,  // note!
          .username = settings_.fix.username,
          .password = signature_,
      };
    }
  }
  std::abort();
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
