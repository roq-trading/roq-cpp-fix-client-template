/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/samples/fix_client/strategy.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {

// === CONSTANTS ===

namespace {
auto const TIMER_FREQUENCY = 100ms;
}

// === IMPLEMENTATION ===

Strategy::Strategy(Settings const &settings, io::Context &context, io::web::URI const &uri)
    : settings_{settings}, context_{context},
      interrupt_{context.create_signal(*this, io::sys::Signal::Type::INTERRUPT)},
      timer_{context.create_timer(*this, TIMER_FREQUENCY)}, session_{*this, settings, context, shared_, uri} {
}

void Strategy::dispatch() {
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

void Strategy::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, magic_enum::enum_name(event.type));
  context_.stop();
}

// io::sys::Timer::Handler

void Strategy::operator()(io::sys::Timer::Event const &event) {
  auto timer = Timer{
      .now = event.now,
  };
  dispatch(timer);
}

// Session::Handler

void Strategy::operator()(Trace<Session::Ready> const &) {
}

void Strategy::operator()(Trace<Session::Disconnected> const &) {
}

//

void Strategy::operator()(Trace<codec::fix::BusinessMessageReject> const &) {
}

// user

void Strategy::operator()(Trace<codec::fix::UserResponse> const &) {
}

// security

void Strategy::operator()(Trace<codec::fix::SecurityList> const &) {
}

void Strategy::operator()(Trace<codec::fix::SecurityDefinition> const &) {
}

void Strategy::operator()(Trace<codec::fix::SecurityStatus> const &) {
}

// market data

void Strategy::operator()(Trace<codec::fix::MarketDataRequestReject> const &) {
}

void Strategy::operator()(Trace<codec::fix::MarketDataSnapshotFullRefresh> const &) {
}

void Strategy::operator()(Trace<codec::fix::MarketDataIncrementalRefresh> const &) {
}

// orders

void Strategy::operator()(Trace<codec::fix::OrderCancelReject> const &) {
}

void Strategy::operator()(Trace<codec::fix::OrderMassCancelReport> const &) {
}

void Strategy::operator()(Trace<codec::fix::ExecutionReport> const &) {
}

// positions

void Strategy::operator()(Trace<codec::fix::RequestForPositionsAck> const &) {
}

void Strategy::operator()(Trace<codec::fix::PositionReport> const &) {
}

// trades

void Strategy::operator()(Trace<codec::fix::TradeCaptureReportRequestAck> const &) {
}

void Strategy::operator()(Trace<codec::fix::TradeCaptureReport> const &) {
}

// utilities

template <typename... Args>
void Strategy::dispatch(Args &&...args) {
  MessageInfo message_info;
  Event event{message_info, std::forward<Args>(args)...};
  session_(event);
}

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
