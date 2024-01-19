/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/io/context.hpp"

#include "roq/io/sys/signal.hpp"
#include "roq/io/sys/timer.hpp"

#include "roq/io/web/uri.hpp"

#include "roq/samples/fix_client/session.hpp"
#include "roq/samples/fix_client/settings.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Strategy final : public io::sys::Signal::Handler, public io::sys::Timer::Handler, public Session::Handler {
  Strategy(Settings const &, io::Context &, io::web::URI const &);

  // Strategy(Strategy &&) = default;
  Strategy(Strategy const &) = delete;

  void dispatch();

 protected:
  // io::sys::Signal::Handler
  void operator()(io::sys::Signal::Event const &) override;

  // io::sys::Timer::Handler
  void operator()(io::sys::Timer::Event const &) override;

  // Session::Handler
  void operator()(Trace<Session::Ready> const &) override;
  void operator()(Trace<Session::Disconnected> const &) override;
  //
  void operator()(Trace<codec::fix::BusinessMessageReject> const &) override;
  // user
  void operator()(Trace<codec::fix::UserResponse> const &) override;
  // security
  void operator()(Trace<codec::fix::SecurityList> const &) override;
  void operator()(Trace<codec::fix::SecurityDefinition> const &) override;
  void operator()(Trace<codec::fix::SecurityStatus> const &) override;
  // market data
  void operator()(Trace<codec::fix::MarketDataRequestReject> const &) override;
  void operator()(Trace<codec::fix::MarketDataSnapshotFullRefresh> const &) override;
  void operator()(Trace<codec::fix::MarketDataIncrementalRefresh> const &) override;
  // orders
  void operator()(Trace<codec::fix::OrderCancelReject> const &) override;
  void operator()(Trace<codec::fix::OrderMassCancelReport> const &) override;
  void operator()(Trace<codec::fix::ExecutionReport> const &) override;
  // positions
  void operator()(Trace<codec::fix::RequestForPositionsAck> const &) override;
  void operator()(Trace<codec::fix::PositionReport> const &) override;
  // trades
  void operator()(Trace<codec::fix::TradeCaptureReportRequestAck> const &) override;
  void operator()(Trace<codec::fix::TradeCaptureReport> const &) override;

  // utilities
  template <typename... Args>
  void dispatch(Args &&...);

 private:
  Settings const &settings_;
  io::Context &context_;
  std::unique_ptr<io::sys::Signal> terminate_;
  std::unique_ptr<io::sys::Signal> interrupt_;
  std::unique_ptr<io::sys::Timer> timer_;
  Shared shared_;
  Session session_;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
