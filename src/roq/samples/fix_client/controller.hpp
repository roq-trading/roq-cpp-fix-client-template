/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/io/context.hpp"

#include "roq/io/sys/signal.hpp"
#include "roq/io/sys/timer.hpp"

#include "roq/io/web/uri.hpp"

#include "roq/samples/fix_client/settings.hpp"
#include "roq/samples/fix_client/shared.hpp"

#include "roq/samples/fix_client/service/manager.hpp"

#include "roq/fix/client/manager.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Controller final : public io::sys::Signal::Handler,
                          public io::sys::Timer::Handler,
                          public service::Manager::Handler,
                          public fix::client::Manager::Handler {
  Controller(Settings const &, io::Context &, io::web::URI const &);

  Controller(Controller const &) = delete;

  void dispatch();

 protected:
  // io::sys::Signal::Handler
  void operator()(io::sys::Signal::Event const &) override;

  // io::sys::Timer::Handler
  void operator()(io::sys::Timer::Event const &) override;

  // fix::client::Manager::Handler
  void operator()(Trace<fix::client::Manager::Connected> const &) override;
  void operator()(Trace<fix::client::Manager::Disconnected> const &) override;
  void operator()(Trace<fix::client::Manager::Ready> const &) override;
  // - business
  void operator()(Trace<fix::codec::BusinessMessageReject> const &) override;
  // - user
  void operator()(Trace<fix::codec::UserResponse> const &) override;
  // - security
  void operator()(Trace<fix::codec::SecurityList> const &) override;
  void operator()(Trace<fix::codec::SecurityDefinition> const &) override;
  void operator()(Trace<fix::codec::SecurityStatus> const &) override;
  // - market data
  void operator()(Trace<fix::codec::MarketDataRequestReject> const &) override;
  void operator()(Trace<fix::codec::MarketDataSnapshotFullRefresh> const &) override;
  void operator()(Trace<fix::codec::MarketDataIncrementalRefresh> const &) override;
  // - orders
  void operator()(Trace<fix::codec::OrderCancelReject> const &) override;
  void operator()(Trace<fix::codec::OrderMassCancelReport> const &) override;
  void operator()(Trace<fix::codec::ExecutionReport> const &) override;
  // - positions
  void operator()(Trace<fix::codec::RequestForPositionsAck> const &) override;
  void operator()(Trace<fix::codec::PositionReport> const &) override;
  // - trades
  void operator()(Trace<fix::codec::TradeCaptureReportRequestAck> const &) override;
  void operator()(Trace<fix::codec::TradeCaptureReport> const &) override;

  // service::Manager::Handler
  void operator()(metrics::Writer &) override;

  // helpers
  template <typename... Args>
  void dispatch(Args &&...);

 private:
  io::Context &context_;
  std::unique_ptr<io::sys::Signal> terminate_;
  std::unique_ptr<io::sys::Signal> interrupt_;
  std::unique_ptr<io::sys::Timer> timer_;
  Shared shared_;
  std::unique_ptr<service::Manager> service_manager_;
  std::unique_ptr<fix::client::Manager> session_manager_;
  // EXPERIMENTAL
  std::chrono::nanoseconds request_time_ = {};
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
