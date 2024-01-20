/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/samples/fix_client/crypto.hpp"

#include "roq/logging.hpp"

#include "roq/utils/codec/base64.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

// === IMPLEMENTATION ===

Crypto::Crypto(Settings const &settings) : settings_{settings} {
}

codec::fix::Logon Crypto::create_logon() {
  auto heart_bt_int = static_cast<decltype(codec::fix::Logon::heart_bt_int)>(
      std::chrono::duration_cast<std::chrono::seconds>(settings_.fix.ping_freq).count());
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
  /*
  if (simple_)
    return password == secret;
  MAC mac{secret};  // alloc
  // mac.clear();
  mac.update(raw_data);
  auto digest = mac.final(digest_);
  std::string result;
  utils::codec::Base64::encode(result, digest, false, false);  // alloc
  log::warn("{}"sv, result);
  return result == password;
  */
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
