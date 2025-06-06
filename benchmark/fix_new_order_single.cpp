/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <benchmark/benchmark.h>

#include "roq/fix/codec/new_order_single.hpp"

using namespace roq;

using namespace std::literals;
using namespace std::chrono_literals;

namespace {
using Header = fix::Header;
using NewOrderSingle = fix::codec::NewOrderSingle;
};  // namespace

void BM_fix_new_order_single_create_message(benchmark::State &state) {
  std::vector<std::byte> buffer(4096);
  for (auto _ : state) {
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
    [[maybe_unused]] auto message = new_order_single.encode(header, buffer);
  }
}

BENCHMARK(BM_fix_new_order_single_create_message);
