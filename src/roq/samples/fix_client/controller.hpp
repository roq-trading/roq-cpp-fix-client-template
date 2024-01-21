/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/io/context.hpp"

#include "roq/io/sys/signal.hpp"
#include "roq/io/sys/timer.hpp"

#include "roq/io/web/uri.hpp"

#include "roq/samples/fix_client/settings.hpp"
#include "roq/samples/fix_client/shared.hpp"

#include "roq/samples/fix_client/session/manager.hpp"

#include "roq/samples/fix_client/service/manager.hpp"

namespace roq {
namespace samples {
namespace fix_client {

struct Controller final : public io::sys::Signal::Handler,
                          public io::sys::Timer::Handler,
                          public session::Manager::Handler,
                          public service::Manager::Handler {
  Controller(Settings const &, io::Context &, io::web::URI const &);

  Controller(Controller const &) = delete;

  void dispatch();

 protected:
  // io::sys::Signal::Handler
  void operator()(io::sys::Signal::Event const &) override;

  // io::sys::Timer::Handler
  void operator()(io::sys::Timer::Event const &) override;

  // session::Manager::Handler
  void operator()(Trace<session::Manager::Ready> const &) override;
  void operator()(Trace<session::Manager::Disconnected> const &) override;
  // - business
  void operator()(Trace<codec::fix::BusinessMessageReject> const &) override;
  // - user
  void operator()(Trace<codec::fix::UserResponse> const &) override;
  // - security
  void operator()(Trace<codec::fix::SecurityList> const &) override;
  void operator()(Trace<codec::fix::SecurityDefinition> const &) override;
  void operator()(Trace<codec::fix::SecurityStatus> const &) override;
  // - market data
  void operator()(Trace<codec::fix::MarketDataRequestReject> const &) override;
  void operator()(Trace<codec::fix::MarketDataSnapshotFullRefresh> const &) override;
  void operator()(Trace<codec::fix::MarketDataIncrementalRefresh> const &) override;
  // - orders
  void operator()(Trace<codec::fix::OrderCancelReject> const &) override;
  void operator()(Trace<codec::fix::OrderMassCancelReport> const &) override;
  void operator()(Trace<codec::fix::ExecutionReport> const &) override;
  // - positions
  void operator()(Trace<codec::fix::RequestForPositionsAck> const &) override;
  void operator()(Trace<codec::fix::PositionReport> const &) override;
  // - trades
  void operator()(Trace<codec::fix::TradeCaptureReportRequestAck> const &) override;
  void operator()(Trace<codec::fix::TradeCaptureReport> const &) override;

  // service::Manager::Handler
  // ---

  // helpers
  template <typename... Args>
  void dispatch(Args &&...);

 private:
  io::Context &context_;
  std::unique_ptr<io::sys::Signal> terminate_;
  std::unique_ptr<io::sys::Signal> interrupt_;
  std::unique_ptr<io::sys::Timer> timer_;
  Shared shared_;
  session::Manager session_manager_;
  std::unique_ptr<service::Manager> service_manager_;
};

}  // namespace fix_client
}  // namespace samples
}  // namespace roq
