/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/fix_client/session/manager.hpp"

#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <nameof.hpp>

#include "roq/logging.hpp"

#include "roq/exceptions.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/debug/fix/message.hpp"
#include "roq/utils/debug/hex/message.hpp"

#include "roq/fix/reader.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {
namespace session {

// === CONSTANTS ===

namespace {
auto const FIX_VERSION = roq::fix::Version::FIX_44;
auto const LOGOUT_RESPONSE = "LOGOUT"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_connection_factory(auto &context, auto &uri) {
  log::debug("uri={}"sv, uri);
  auto config = io::net::ConnectionFactory::Config{
      .interface = {},
      .uris = {&uri, 1},
      .validate_certificate = {},
  };
  return io::net::ConnectionFactory::create(context, config);
}

auto create_connection_manager(auto &handler, auto &settings, auto &connection_factory) {
  auto config = io::net::ConnectionManager::Config{
      .connection_timeout = settings.fix.request_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
  };
  return io::net::ConnectionManager::create(handler, connection_factory, config);
}
}  // namespace

// === IMPLEMENTATION ===

Manager::Manager(Handler &handler, Settings const &settings, io::Context &context, io::web::URI const &uri)
    : handler_{handler}, settings_{settings}, crypto_{settings}, connection_factory_{create_connection_factory(context, uri)},
      connection_manager_{create_connection_manager(*this, settings, *connection_factory_)}, decode_buffer_(settings.fix.decode_buffer_size),
      decode_buffer_2_(settings.fix.decode_buffer_size) {
}

void Manager::operator()(Event<Start> const &) {
  (*connection_manager_).start();
}

void Manager::operator()(Event<Stop> const &) {
  (*connection_manager_).stop();
}

void Manager::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_manager_).refresh(now);
  if (state_ <= State::LOGON_SENT) {
    return;
  }
  if (next_heartbeat_ <= now) {
    next_heartbeat_ = now + settings_.fix.ping_freq;
    send_test_request(now);
  }
}

void Manager::operator()(Manager::State state) {
  if (utils::update(state_, state)) {
    log::debug("state={}"sv, state);
  }
}

// outbound

// - user

void Manager::operator()(fix::codec::UserRequest const &value) {
  send(value);
}

// - security

void Manager::operator()(fix::codec::SecurityListRequest const &value) {
  send(value);
}

void Manager::operator()(fix::codec::SecurityDefinitionRequest const &value) {
  send(value);
}

void Manager::operator()(fix::codec::SecurityStatusRequest const &value) {
  send(value);
}

// - market data

void Manager::operator()(fix::codec::MarketDataRequest const &value) {
  send(value);
}

// - orders

void Manager::operator()(fix::codec::OrderStatusRequest const &value) {
  send(value);
}

void Manager::operator()(fix::codec::NewOrderSingle const &value) {
  log::debug("new_order_single={}"sv, value);
  send(value);
}

void Manager::operator()(fix::codec::OrderCancelReplaceRequest const &value) {
  log::debug("order_cancel_replace_request={}"sv, value);
  send(value);
}

void Manager::operator()(fix::codec::OrderCancelRequest const &value) {
  log::debug("order_cancel_request={}"sv, value);
  send(value);
}

void Manager::operator()(fix::codec::OrderMassStatusRequest const &value) {
  send(value);
}

void Manager::operator()(fix::codec::OrderMassCancelRequest const &value) {
  log::debug("order_cancel_request={}"sv, value);
  send(value);
}

// - positions

void Manager::operator()(fix::codec::RequestForPositions const &value) {
  send(value);
}

// - trades

void Manager::operator()(fix::codec::TradeCaptureReportRequest const &value) {
  send(value);
}

// io::net::ConnectionManager::Handler

void Manager::operator()(io::net::ConnectionManager::Connected const &) {
  log::debug("Connected"sv);
  send_logon();
  (*this)(State::LOGON_SENT);
}

void Manager::operator()(io::net::ConnectionManager::Disconnected const &) {
  log::debug("Disconnected"sv);
  TraceInfo trace_info;
  Disconnected disconnected;
  Trace event{trace_info, disconnected};
  handler_(event);
  outbound_ = {};
  inbound_ = {};
  next_heartbeat_ = {};
  (*this)(State::DISCONNECTED);
}

void Manager::operator()(io::net::ConnectionManager::Read const &) {
  auto logger = [this](auto &message) {
    if (settings_.fix.debug) [[unlikely]] {
      log::info("{}"sv, utils::debug::fix::Message{message});
    }
  };
  auto buffer = (*connection_manager_).buffer();
  size_t total_bytes = 0;
  while (!std::empty(buffer)) {
    TraceInfo trace_info;
    auto parser = [&](auto &message) {
      try {
        check(message.header);
        Trace event{trace_info, message};
        parse(event);
      } catch (std::exception &) {
        log::warn("{}"sv, utils::debug::fix::Message{buffer});
#ifndef NDEBUG
        log::warn("{}"sv, utils::debug::hex::Message{buffer});
#endif
        log::error("Message could not be parsed. PLEASE REPORT!"sv);
        throw;
      }
    };
    auto bytes = roq::fix::Reader<FIX_VERSION>::dispatch(buffer, parser, logger);
    if (bytes == 0) {
      break;
    }
    assert(bytes <= std::size(buffer));
    total_bytes += bytes;
    buffer = buffer.subspan(bytes);
  }
  (*connection_manager_).drain(total_bytes);
}

void Manager::operator()(io::net::ConnectionManager::Write const &) {
}

// inbound

void Manager::check(roq::fix::Header const &header) {
  auto current = header.msg_seq_num;
  auto expected = inbound_.msg_seq_num + 1;
  if (current != expected) [[unlikely]] {
    if (expected < current) {
      log::warn(
          "*** SEQUENCE GAP *** "
          "current={} previous={} distance={}"sv,
          current,
          inbound_.msg_seq_num,
          current - inbound_.msg_seq_num);
    } else {
      log::warn(
          "*** SEQUENCE REPLAY *** "
          "current={} previous={} distance={}"sv,
          current,
          inbound_.msg_seq_num,
          inbound_.msg_seq_num - current);
    }
  }
  inbound_.msg_seq_num = current;
}

void Manager::parse(Trace<roq::fix::Message> const &event) {
  auto &[trace_info, message] = event;
  auto &header = message.header;
  switch (header.msg_type) {
    using enum roq::fix::MsgType;
    // - session
    case REJECT: {
      auto reject = fix::codec::Reject::create(message);
      dispatch(event, reject);
      break;
    }
    case RESEND_REQUEST: {
      auto resend_request = fix::codec::ResendRequest::create(message);
      dispatch(event, resend_request);
      break;
    }
    case LOGON: {
      auto logon = fix::codec::Logon::create(message);
      dispatch(event, logon);
      break;
    }
    case LOGOUT: {
      auto logout = fix::codec::Heartbeat::create(message);
      dispatch(event, logout);
      break;
    }
    case HEARTBEAT: {
      auto heartbeat = fix::codec::Heartbeat::create(message);
      dispatch(event, heartbeat);
      break;
    }
    case TEST_REQUEST: {
      auto test_request = fix::codec::TestRequest::create(message);
      dispatch(event, test_request);
      break;
    }
    // - business
    case BUSINESS_MESSAGE_REJECT: {
      auto business_message_reject = fix::codec::BusinessMessageReject::create(message);
      dispatch(event, business_message_reject);
      break;
    }
    // - user
    case USER_RESPONSE: {
      auto user_response = fix::codec::UserResponse::create(message);
      dispatch(event, user_response);
      break;
    }
    // - security
    case SECURITY_LIST: {
      auto security_list = fix::codec::SecurityList::create(message, decode_buffer_);
      dispatch(event, security_list);
      break;
    }
    case SECURITY_DEFINITION: {
      auto security_definition = fix::codec::SecurityDefinition::create(message, decode_buffer_);
      dispatch(event, security_definition);
      break;
    }
    case SECURITY_STATUS: {
      auto security_status = fix::codec::SecurityStatus::create(message, decode_buffer_);
      dispatch(event, security_status);
      break;
    }
    // - market data
    case MARKET_DATA_REQUEST_REJECT: {
      auto market_data_request_reject = fix::codec::MarketDataRequestReject::create(message, decode_buffer_);
      dispatch(event, market_data_request_reject);
      break;
    }
    case MARKET_DATA_SNAPSHOT_FULL_REFRESH: {
      auto market_data_snapshot_full_refresh = fix::codec::MarketDataSnapshotFullRefresh::create(message, decode_buffer_);
      dispatch(event, market_data_snapshot_full_refresh);
      break;
    }
    case MARKET_DATA_INCREMENTAL_REFRESH: {
      auto market_data_incremental_refresh = fix::codec::MarketDataIncrementalRefresh::create(message, decode_buffer_);
      dispatch(event, market_data_incremental_refresh);
      break;
    }
    // - orders
    case ORDER_CANCEL_REJECT: {
      auto order_cancel_reject = fix::codec::OrderCancelReject::create(message, decode_buffer_);
      dispatch(event, order_cancel_reject);
      break;
    }
    case ORDER_MASS_CANCEL_REPORT: {
      auto order_mass_cancel_report = fix::codec::OrderMassCancelReport::create(message, decode_buffer_);
      dispatch(event, order_mass_cancel_report);
      break;
    }
    case EXECUTION_REPORT: {
      auto execution_report = fix::codec::ExecutionReport::create(message, decode_buffer_);
      dispatch(event, execution_report);
      break;
    }
    // - positions
    case REQUEST_FOR_POSITIONS_ACK: {
      auto request_for_positions_ack = fix::codec::RequestForPositionsAck::create(message, decode_buffer_);
      dispatch(event, request_for_positions_ack);
      break;
    }
    case POSITION_REPORT: {
      auto position_report = fix::codec::PositionReport::create(message, decode_buffer_);
      dispatch(event, position_report);
      break;
    }
    // - trades
    case TRADE_CAPTURE_REPORT_REQUEST_ACK: {
      auto trade_capture_report_request_ack = fix::codec::TradeCaptureReportRequestAck::create(message, decode_buffer_);
      dispatch(event, trade_capture_report_request_ack);
      break;
    }
    case TRADE_CAPTURE_REPORT: {
      auto trade_capture_report = fix::codec::TradeCaptureReport::create(message, decode_buffer_, decode_buffer_2_);
      dispatch(event, trade_capture_report);
      break;
    }
    default:
      log::warn("Unsupported: msg_type={}"sv, header.msg_type);
  }
}

template <typename T>
void Manager::dispatch(Trace<roq::fix::Message> const &event, T const &value) {
  auto &[trace_info, message] = event;
  log::info<1>("{}={}"sv, nameof::nameof_short_type<T>(), value);
  Trace event_2{trace_info, value};
  (*this)(event_2, message.header);
}

// - session

void Manager::operator()(Trace<fix::codec::Reject> const &event, roq::fix::Header const &) {
  auto &[trace_info, reject] = event;
  log::debug("reject={}, trace_info={}"sv, reject, trace_info);
}

void Manager::operator()(Trace<fix::codec::ResendRequest> const &event, roq::fix::Header const &) {
  auto &[trace_info, resend_request] = event;
  log::debug("resend_request={}, trace_info={}"sv, resend_request, trace_info);
}

void Manager::operator()(Trace<fix::codec::Logon> const &event, roq::fix::Header const &) {
  auto &[trace_info, logon] = event;
  log::debug("logon={}, trace_info={}"sv, logon, trace_info);
  assert(state_ == State::LOGON_SENT);
  (*this)(State::READY);
  Ready ready;
  Trace event_2{trace_info, ready};
  handler_(event_2);
}

void Manager::operator()(Trace<fix::codec::Logout> const &event, roq::fix::Header const &) {
  auto &[trace_info, logout] = event;
  log::debug("logout={}, trace_info={}"sv, logout, trace_info);
  // note! mandated, must send a logout response
  send_logout(LOGOUT_RESPONSE);
  log::warn("closing connection"sv);
  (*connection_manager_).close();
}

void Manager::operator()(Trace<fix::codec::Heartbeat> const &event, roq::fix::Header const &) {
  auto &[trace_info, heartbeat] = event;
  log::debug("heartbeat={}, trace_info={}"sv, heartbeat, trace_info);
}

void Manager::operator()(Trace<fix::codec::TestRequest> const &event, roq::fix::Header const &) {
  auto &[trace_info, test_request] = event;
  send_heartbeat(test_request.test_req_id);
}

// - business

void Manager::operator()(Trace<fix::codec::BusinessMessageReject> const &event, roq::fix::Header const &) {
  auto &[trace_info, business_message_reject] = event;
  log::debug("business_message_reject={}, trace_info={}"sv, business_message_reject, trace_info);
  handler_(event);
}

// - user

void Manager::operator()(Trace<fix::codec::UserResponse> const &event, roq::fix::Header const &) {
  auto &[trace_info, user_response] = event;
  log::debug("user_response={}, trace_info={}"sv, user_response, trace_info);
  handler_(event);
}

// - security

void Manager::operator()(Trace<fix::codec::SecurityList> const &event, roq::fix::Header const &) {
  auto &[trace_info, security_list] = event;
  log::debug("security_list={}, trace_info={}"sv, security_list, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::SecurityDefinition> const &event, roq::fix::Header const &) {
  auto &[trace_info, security_definition] = event;
  log::debug("security_definition={}, trace_info={}"sv, security_definition, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::SecurityStatus> const &event, roq::fix::Header const &) {
  auto &[trace_info, security_status] = event;
  log::debug("security_status={}, trace_info={}"sv, security_status, trace_info);
  handler_(event);
}

// - market data

void Manager::operator()(Trace<fix::codec::MarketDataRequestReject> const &event, roq::fix::Header const &) {
  auto &[trace_info, market_data_request_reject] = event;
  log::debug("market_data_request_reject={}, trace_info={}"sv, market_data_request_reject, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::MarketDataSnapshotFullRefresh> const &event, roq::fix::Header const &) {
  auto &[trace_info, market_data_snapshot_full_refresh] = event;
  log::debug<1>("market_data_snapshot_full_refresh={}, trace_info={}"sv, market_data_snapshot_full_refresh, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::MarketDataIncrementalRefresh> const &event, roq::fix::Header const &) {
  auto &[trace_info, market_data_incremental_refresh] = event;
  log::debug<1>("market_data_incremental_refresh={}, trace_info={}"sv, market_data_incremental_refresh, trace_info);
  handler_(event);
}

// - order

void Manager::operator()(Trace<fix::codec::OrderCancelReject> const &event, roq::fix::Header const &) {
  auto &[trace_info, order_cancel_reject] = event;
  log::debug("order_cancel_reject={}, trace_info={}"sv, order_cancel_reject, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::OrderMassCancelReport> const &event, roq::fix::Header const &) {
  auto &[trace_info, order_mass_cancel_report] = event;
  log::debug("order_mass_cancel_report={}, trace_info={}"sv, order_mass_cancel_report, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::ExecutionReport> const &event, roq::fix::Header const &) {
  auto &[trace_info, execution_report] = event;
  log::debug("execution_report={}, trace_info={}"sv, execution_report, trace_info);
  handler_(event);
}

// - positions

void Manager::operator()(Trace<fix::codec::RequestForPositionsAck> const &event, roq::fix::Header const &) {
  auto &[trace_info, request_for_positions_ack] = event;
  log::debug("request_for_positions_ack={}, trace_info={}"sv, request_for_positions_ack, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::PositionReport> const &event, roq::fix::Header const &) {
  auto &[trace_info, position_report] = event;
  log::debug("position_report={}, trace_info={}"sv, position_report, trace_info);
  handler_(event);
}

// - trades

void Manager::operator()(Trace<fix::codec::TradeCaptureReportRequestAck> const &event, roq::fix::Header const &) {
  auto &[trace_info, trade_capture_report_request_ack] = event;
  log::debug("trade_capture_report_request_ack={}, trace_info={}"sv, trade_capture_report_request_ack, trace_info);
  handler_(event);
}

void Manager::operator()(Trace<fix::codec::TradeCaptureReport> const &event, roq::fix::Header const &) {
  auto &[trace_info, trade_capture_report] = event;
  log::debug("trade_capture_report={}, trace_info={}"sv, trade_capture_report, trace_info);
  handler_(event);
}

// outbound

void Manager::send(auto const &value) {
  send(value, clock::get_realtime());
}

template <typename T>
void Manager::send(T const &value, std::chrono::nanoseconds sending_time_utc) {
  if constexpr (utils::is_specialization<T, Trace>::value) {
    // external
    if (!ready()) {
      throw NotReady{"not ready"sv};
    }
    send_helper(value.value, sending_time_utc);
  } else {
    // internal
    send_helper(value, sending_time_utc);
  }
}

template <typename T>
void Manager::send_helper(T const &value, std::chrono::nanoseconds sending_time_utc) {
  log::info<2>("send (=> server): {}={}"sv, nameof::nameof_short_type<T>(), value);
  auto header = roq::fix::Header{
      .version = FIX_VERSION,
      .msg_type = T::MSG_TYPE,
      .sender_comp_id = settings_.fix.sender_comp_id,
      .target_comp_id = settings_.fix.target_comp_id,
      .msg_seq_num = ++outbound_.msg_seq_num,  // note!
      .sending_time = sending_time_utc,
  };
  if ((*connection_manager_).send([&](auto &buffer) {
        auto message = value.encode(header, buffer);
        if (settings_.fix.debug) [[unlikely]] {
          log::info("{}"sv, utils::debug::fix::Message{message});
        }
        return std::size(message);
      })) {
  } else {
    log::warn("HERE"sv);
  }
}

void Manager::send_logon() {
  auto now = clock::get_realtime();
  auto logon = crypto_.create_logon(now);
  log::debug("logon={}"sv, logon);
  send(logon, now);
}

void Manager::send_logout(std::string_view const &text) {
  auto logout = fix::codec::Logout{
      .text = text,
  };
  send(logout);
}

void Manager::send_heartbeat(std::string_view const &test_req_id) {
  auto heartbeat = fix::codec::Heartbeat{
      .test_req_id = test_req_id,
  };
  send(heartbeat);
}

void Manager::send_test_request(std::chrono::nanoseconds now) {
  auto test_req_id = fmt::format("{}"sv, now.count());
  auto test_request = fix::codec::TestRequest{
      .test_req_id = test_req_id,
  };
  send(test_request);
}

}  // namespace session
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
