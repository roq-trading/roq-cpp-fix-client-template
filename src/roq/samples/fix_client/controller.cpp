/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/samples/fix_client/controller.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

// === CONSTANTS ===

namespace {
auto const TIMER_FREQUENCY = 100ms;

auto const EXCHANGE = "deribit"sv;
auto const SYMBOL = "BTC-PERPETUAL"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_service_manager(auto &handler, auto &settings, auto &context) -> std::unique_ptr<service::Manager> {
  if (std::empty(settings.service.listen_address))
    return {};
  return std::make_unique<service::Manager>(handler, settings, context);
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, io::Context &context, io::web::URI const &uri)
    : settings_{settings}, context_{context},
      interrupt_{context.create_signal(*this, io::sys::Signal::Type::INTERRUPT)},
      timer_{context.create_timer(*this, TIMER_FREQUENCY)}, session_{*this, settings, context, uri},
      service_manager_{create_service_manager(*this, settings, context)} {
}

void Controller::dispatch() {
  log::info("Event loop is now running"sv);
  Start start;
  dispatch(start);
  (*timer_).resume();
  context_.dispatch();
  Stop stop;
  dispatch(stop);
  log::info("Event loop has terminated"sv);
}

// io::sys::Signal::Handler

void Controller::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, magic_enum::enum_name(event.type));
  context_.stop();
}

// io::sys::Timer::Handler

void Controller::operator()(io::sys::Timer::Event const &event) {
  auto timer = Timer{
      .now = event.now,
  };
  MessageInfo message_info;
  Event event_2{message_info, timer};
  session_(event_2);
  if (service_manager_)
    (*service_manager_)(event_2);
}

// Session::Handler

void Controller::operator()(Trace<Session::Ready> const &) {
  auto security_definition_request = codec::fix::SecurityDefinitionRequest{
      .security_req_id = "test"sv,
      .security_request_type = roq::fix::SecurityRequestType::REQUEST_LIST_SECURITIES,
      .symbol = SYMBOL,
      .security_exchange = EXCHANGE,
      .trading_session_id = {},
      .subscription_request_type = roq::fix::SubscriptionRequestType::SNAPSHOT_UPDATES,
  };
  session_(security_definition_request);
}

void Controller::operator()(Trace<Session::Disconnected> const &) {
}

//

void Controller::operator()(Trace<codec::fix::BusinessMessageReject> const &) {
}

// user

void Controller::operator()(Trace<codec::fix::UserResponse> const &) {
}

// security

void Controller::operator()(Trace<codec::fix::SecurityList> const &) {
}

void Controller::operator()(Trace<codec::fix::SecurityDefinition> const &) {
}

void Controller::operator()(Trace<codec::fix::SecurityStatus> const &) {
}

// market data

void Controller::operator()(Trace<codec::fix::MarketDataRequestReject> const &) {
}

void Controller::operator()(Trace<codec::fix::MarketDataSnapshotFullRefresh> const &) {
}

void Controller::operator()(Trace<codec::fix::MarketDataIncrementalRefresh> const &) {
}

// orders

void Controller::operator()(Trace<codec::fix::OrderCancelReject> const &) {
}

void Controller::operator()(Trace<codec::fix::OrderMassCancelReport> const &) {
}

void Controller::operator()(Trace<codec::fix::ExecutionReport> const &) {
}

// positions

void Controller::operator()(Trace<codec::fix::RequestForPositionsAck> const &) {
}

void Controller::operator()(Trace<codec::fix::PositionReport> const &) {
}

// trades

void Controller::operator()(Trace<codec::fix::TradeCaptureReportRequestAck> const &) {
}

void Controller::operator()(Trace<codec::fix::TradeCaptureReport> const &) {
}

// utilities

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  MessageInfo message_info;
  Event event{message_info, std::forward<Args>(args)...};
  session_(event);
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
