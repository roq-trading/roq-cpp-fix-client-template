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
}

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, io::Context &context, io::web::URI const &uri)
    : settings_{settings}, context_{context},
      interrupt_{context.create_signal(*this, io::sys::Signal::Type::INTERRUPT)},
      timer_{context.create_timer(*this, TIMER_FREQUENCY)}, session_{*this, settings, context, shared_, uri} {
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
  dispatch(timer);
}

// Session::Handler

void Controller::operator()(Trace<Session::Ready> const &) {
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
