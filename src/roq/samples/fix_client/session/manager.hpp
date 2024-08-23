/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "roq/api.hpp"

#include "roq/io/context.hpp"

#include "roq/io/web/uri.hpp"

#include "roq/io/net/connection_factory.hpp"
#include "roq/io/net/connection_manager.hpp"

#include "roq/fix/message.hpp"

#include "roq/codec/fix/business_message_reject.hpp"
#include "roq/codec/fix/execution_report.hpp"
#include "roq/codec/fix/heartbeat.hpp"
#include "roq/codec/fix/logon.hpp"
#include "roq/codec/fix/logout.hpp"
#include "roq/codec/fix/market_data_incremental_refresh.hpp"
#include "roq/codec/fix/market_data_request.hpp"
#include "roq/codec/fix/market_data_request_reject.hpp"
#include "roq/codec/fix/market_data_snapshot_full_refresh.hpp"
#include "roq/codec/fix/new_order_single.hpp"
#include "roq/codec/fix/order_cancel_reject.hpp"
#include "roq/codec/fix/order_cancel_replace_request.hpp"
#include "roq/codec/fix/order_cancel_request.hpp"
#include "roq/codec/fix/order_mass_cancel_report.hpp"
#include "roq/codec/fix/order_mass_cancel_request.hpp"
#include "roq/codec/fix/order_mass_status_request.hpp"
#include "roq/codec/fix/order_status_request.hpp"
#include "roq/codec/fix/position_report.hpp"
#include "roq/codec/fix/reject.hpp"
#include "roq/codec/fix/request_for_positions.hpp"
#include "roq/codec/fix/request_for_positions_ack.hpp"
#include "roq/codec/fix/resend_request.hpp"
#include "roq/codec/fix/security_definition.hpp"
#include "roq/codec/fix/security_definition_request.hpp"
#include "roq/codec/fix/security_list.hpp"
#include "roq/codec/fix/security_list_request.hpp"
#include "roq/codec/fix/security_status.hpp"
#include "roq/codec/fix/security_status_request.hpp"
#include "roq/codec/fix/test_request.hpp"
#include "roq/codec/fix/trade_capture_report.hpp"
#include "roq/codec/fix/trade_capture_report_request.hpp"
#include "roq/codec/fix/trade_capture_report_request_ack.hpp"
#include "roq/codec/fix/user_request.hpp"
#include "roq/codec/fix/user_response.hpp"

#include "roq/samples/fix_client/settings.hpp"

#include "roq/samples/fix_client/session/crypto.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace session {

struct Manager final : public io::net::ConnectionManager::Handler {
  struct Ready final {};
  struct Disconnected final {};
  struct Handler {
    virtual void operator()(Trace<Ready> const &) = 0;
    virtual void operator()(Trace<Disconnected> const &) = 0;
    // - business
    virtual void operator()(Trace<codec::fix::BusinessMessageReject> const &) = 0;
    // - user
    virtual void operator()(Trace<codec::fix::UserResponse> const &) = 0;
    // - security
    virtual void operator()(Trace<codec::fix::SecurityList> const &) = 0;
    virtual void operator()(Trace<codec::fix::SecurityDefinition> const &) = 0;
    virtual void operator()(Trace<codec::fix::SecurityStatus> const &) = 0;
    // - market data
    virtual void operator()(Trace<codec::fix::MarketDataRequestReject> const &) = 0;
    virtual void operator()(Trace<codec::fix::MarketDataSnapshotFullRefresh> const &) = 0;
    virtual void operator()(Trace<codec::fix::MarketDataIncrementalRefresh> const &) = 0;
    // - orders
    virtual void operator()(Trace<codec::fix::OrderCancelReject> const &) = 0;
    virtual void operator()(Trace<codec::fix::OrderMassCancelReport> const &) = 0;
    virtual void operator()(Trace<codec::fix::ExecutionReport> const &) = 0;
    // - positions
    virtual void operator()(Trace<codec::fix::RequestForPositionsAck> const &) = 0;
    virtual void operator()(Trace<codec::fix::PositionReport> const &) = 0;
    // - trades
    virtual void operator()(Trace<codec::fix::TradeCaptureReportRequestAck> const &) = 0;
    virtual void operator()(Trace<codec::fix::TradeCaptureReport> const &) = 0;
  };

  Manager(Handler &, Settings const &, io::Context &, io::web::URI const &);

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  bool ready() const { return state_ == State::READY; }

  // outbound

  // - user

  void operator()(codec::fix::UserRequest const &);

  // - security

  void operator()(codec::fix::SecurityListRequest const &);
  void operator()(codec::fix::SecurityDefinitionRequest const &);
  void operator()(codec::fix::SecurityStatusRequest const &);

  // - market data

  void operator()(codec::fix::MarketDataRequest const &);

  // - orders

  void operator()(codec::fix::OrderStatusRequest const &);
  void operator()(codec::fix::NewOrderSingle const &);
  void operator()(codec::fix::OrderCancelReplaceRequest const &);
  void operator()(codec::fix::OrderCancelRequest const &);
  void operator()(codec::fix::OrderMassStatusRequest const &);
  void operator()(codec::fix::OrderMassCancelRequest const &);

  // - positions

  void operator()(codec::fix::RequestForPositions const &);

  // - trades

  void operator()(codec::fix::TradeCaptureReportRequest const &);

 private:
  enum class State;

 protected:
  void operator()(State);

  // io::net::ConnectionManager::Handler
  void operator()(io::net::ConnectionManager::Connected const &) override;
  void operator()(io::net::ConnectionManager::Disconnected const &) override;
  void operator()(io::net::ConnectionManager::Read const &) override;
  void operator()(io::net::ConnectionManager::Write const &) override;

  // inbound

  void check(roq::fix::Header const &);

  void parse(Trace<roq::fix::Message> const &);

  template <typename T>
  void dispatch(Trace<roq::fix::Message> const &, T const &);

  // - session

  void operator()(Trace<codec::fix::Reject> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::ResendRequest> const &, roq::fix::Header const &);

  void operator()(Trace<codec::fix::Logon> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::Logout> const &, roq::fix::Header const &);

  void operator()(Trace<codec::fix::Heartbeat> const &, roq::fix::Header const &);

  void operator()(Trace<codec::fix::TestRequest> const &, roq::fix::Header const &);

  // - business

  void operator()(Trace<codec::fix::BusinessMessageReject> const &, roq::fix::Header const &);

  // - user

  void operator()(Trace<codec::fix::UserResponse> const &, roq::fix::Header const &);

  // - security

  void operator()(Trace<codec::fix::SecurityList> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::SecurityDefinition> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::SecurityStatus> const &, roq::fix::Header const &);

  // - market data

  void operator()(Trace<codec::fix::MarketDataRequestReject> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::MarketDataSnapshotFullRefresh> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::MarketDataIncrementalRefresh> const &, roq::fix::Header const &);

  // - orders

  void operator()(Trace<codec::fix::OrderCancelReject> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::OrderMassCancelReport> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::ExecutionReport> const &, roq::fix::Header const &);

  // - positions

  void operator()(Trace<codec::fix::RequestForPositionsAck> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::PositionReport> const &, roq::fix::Header const &);

  // - trades

  void operator()(Trace<codec::fix::TradeCaptureReportRequestAck> const &, roq::fix::Header const &);
  void operator()(Trace<codec::fix::TradeCaptureReport> const &, roq::fix::Header const &);

  // outbound

  void send(auto const &value);

  template <typename T>
  void send(T const &value, std::chrono::nanoseconds sending_time_utc);

  template <typename T>
  void send_helper(T const &value, std::chrono::nanoseconds sending_time_utc);

  void send_logon();
  void send_logout(std::string_view const &text);
  void send_heartbeat(std::string_view const &test_req_id);
  void send_test_request(std::chrono::nanoseconds now);

 private:
  Handler &handler_;
  Settings const &settings_;
  Crypto crypto_;
  // connection
  std::unique_ptr<io::net::ConnectionFactory> const connection_factory_;
  std::unique_ptr<io::net::ConnectionManager> const connection_manager_;
  // messaging
  struct {
    uint64_t msg_seq_num = {};
  } inbound_;
  struct {
    uint64_t msg_seq_num = {};
  } outbound_;
  std::vector<std::byte> decode_buffer_;
  std::vector<std::byte> decode_buffer_2_;
  // state
  enum class State {
    DISCONNECTED,
    LOGON_SENT,
    READY,
  } state_ = {};
  std::chrono::nanoseconds next_heartbeat_ = {};
};

}  // namespace session
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
