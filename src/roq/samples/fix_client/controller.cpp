/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/fix_client/controller.hpp"

#include <fmt/format.h>

#include <magic_enum/magic_enum_format.hpp>

#include <cassert>

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
  if (std::empty(settings.service.listen_address)) {
    return {};
  }
  return std::make_unique<service::Manager>(handler, settings, context);
}

auto create_session_manager(auto &handler, auto &settings, auto &context, auto &uri) {
  return fix::client::Manager::create(handler, settings, context, uri);
}
}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, io::Context &context, io::web::URI const &uri)
    : context_{context}, interrupt_{context.create_signal(*this, io::sys::Signal::Type::INTERRUPT)}, timer_{context.create_timer(*this, TIMER_FREQUENCY)},
      shared_{settings}, service_manager_{create_service_manager(*this, settings, context)},
      session_manager_{create_session_manager(*this, settings, context, uri)} {
}

void Controller::dispatch() {
  log::info("Event loop is now running"sv);
  (*session_manager_).start();
  (*timer_).resume();
  context_.dispatch();
  (*session_manager_).stop();
  log::info("Event loop has terminated"sv);
}

// io::sys::Signal::Handler

void Controller::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, event.type);
  context_.stop();
}

// io::sys::Timer::Handler

void Controller::operator()(io::sys::Timer::Event const &event) {
  (*session_manager_).refresh(event.now);
  if (service_manager_) {
    auto timer = Timer{
        .now = event.now,
    };
    MessageInfo message_info;
    Event event_2{message_info, timer};
    (*service_manager_)(event_2);
  }
}

// session::Manager::Handler

void Controller::operator()(Trace<fix::client::Manager::Connected> const &) {
}

void Controller::operator()(Trace<fix::client::Manager::Disconnected> const &) {
}

void Controller::operator()(Trace<fix::client::Manager::Ready> const &) {
  request_time_ = clock::get_system();  // note! *before* encoding and sending the request
  auto security_definition_request = fix::codec::SecurityDefinitionRequest{
      .security_req_id = "test"sv,
      .security_request_type = roq::fix::SecurityRequestType::REQUEST_LIST_SECURITIES,
      .symbol = SYMBOL,
      .security_exchange = EXCHANGE,
      .trading_session_id = {},
      .subscription_request_type = roq::fix::SubscriptionRequestType::SNAPSHOT_UPDATES,
  };
  (*session_manager_)(security_definition_request);
}

// - business

void Controller::operator()(Trace<fix::codec::BusinessMessageReject> const &) {
}

// - user

void Controller::operator()(Trace<fix::codec::UserResponse> const &) {
}

// - security

void Controller::operator()(Trace<fix::codec::SecurityList> const &) {
}

void Controller::operator()(Trace<fix::codec::SecurityDefinition> const &) {
  auto now = clock::get_system();  // note! *after* receiving and decoding the response
  auto latency = now - request_time_;
  assert(latency.count() >= 0);
  shared_.request_latency.internal.update(latency.count());  // note! only supporting uint64_t
}

void Controller::operator()(Trace<fix::codec::SecurityStatus> const &) {
}

// - market data

void Controller::operator()(Trace<fix::codec::MarketDataRequestReject> const &) {
}

void Controller::operator()(Trace<fix::codec::MarketDataSnapshotFullRefresh> const &) {
}

void Controller::operator()(Trace<fix::codec::MarketDataIncrementalRefresh> const &) {
}

// - orders

void Controller::operator()(Trace<fix::codec::OrderCancelReject> const &) {
}

void Controller::operator()(Trace<fix::codec::OrderMassCancelReport> const &) {
}

void Controller::operator()(Trace<fix::codec::ExecutionReport> const &) {
}

// - positions

void Controller::operator()(Trace<fix::codec::RequestForPositionsAck> const &) {
}

void Controller::operator()(Trace<fix::codec::PositionReport> const &) {
}

// - trades

void Controller::operator()(Trace<fix::codec::TradeCaptureReportRequestAck> const &) {
}

void Controller::operator()(Trace<fix::codec::TradeCaptureReport> const &) {
}

// service::Manager::Handler

void Controller::operator()(metrics::Writer &writer) {
  shared_(writer);
}

// helpers

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  MessageInfo message_info;
  Event event{message_info, std::forward<Args>(args)...};
  (*session_manager_)(event);
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
