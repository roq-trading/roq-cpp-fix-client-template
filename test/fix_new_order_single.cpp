/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_test_macros.hpp>

#include "roq/utils/debug/fix/message.hpp"
#include "roq/utils/debug/hex/message.hpp"

#include "roq/fix/codec/new_order_single.hpp"

using namespace std::literals;
using namespace std::chrono_literals;

using namespace roq;

namespace {
using Header = fix::Header;
using NewOrderSingle = fix::codec::NewOrderSingle;
};  // namespace

TEST_CASE("fix_new_order_single", "[fix_new_order_single]") {
  std::vector<std::byte> buffer(4096);
  auto new_order_single = NewOrderSingle{
      .cl_ord_id = "123"sv,
      .secondary_cl_ord_id = {},
      .no_party_ids = {},
      .account = "A1",
      .handl_inst = {},
      .exec_inst = {},
      .no_trading_sessions = {},
      .symbol = "BTC-PERPETUAL"sv,
      .security_exchange = "deribit"sv,
      .side = fix::Side::BUY,
      .transact_time = 1685248384123ms,
      .order_qty = {1.0, Precision::_0},
      .ord_type = fix::OrdType::LIMIT,
      .price = {27193.0, Precision::_1},
      .stop_px = {},
      .time_in_force = fix::TimeInForce::GTC,
      .text = {},
      .position_effect = {},
      .max_show = {},
  };
  auto header = Header{
      .version = fix::Version::FIX_44,
      .msg_type = decltype(new_order_single)::MSG_TYPE,
      .sender_comp_id = "sender"sv,
      .target_comp_id = "target"sv,
      .msg_seq_num = 1,
      .sending_time = 1685248384123ms,
  };
  auto message = new_order_single.encode(header, buffer);
  REQUIRE(std::size(message) > 0);
  auto tmp = fmt::format("{}"sv, utils::debug::fix::Message{message});
  // note! you can use https://fixparser.targetcompid.com/ to decode this message
  auto expected =
      "8=FIX.4.4|9=0000152|35=D|49=sender|56=target|34=1|52=20230528-04:33:04.123|"
      "11=123|1=A1|55=BTC-PERPETUAL|207=deribit|54=1|60=20230528-04:33:04.123|38=1|"
      "40=2|44=27193.0|59=1|10=069|"sv;
  CHECK(tmp == expected);
  fmt::print(stderr, "{}\n"sv, utils::debug::hex::Message{message});
}
