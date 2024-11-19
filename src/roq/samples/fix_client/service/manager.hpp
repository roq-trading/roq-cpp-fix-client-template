/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/api.hpp"

#include "roq/metrics/writer.hpp"

#include "roq/utils/container.hpp"

#include "roq/io/context.hpp"

#include "roq/io/net/tcp/listener.hpp"

#include "roq/samples/fix_client/settings.hpp"

#include "roq/samples/fix_client/service/session.hpp"
#include "roq/samples/fix_client/service/shared.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

struct Manager final : public Session::Handler, public io::net::tcp::Listener::Handler {
  struct Handler {
    virtual void operator()(metrics::Writer &) = 0;
  };

  Manager(Handler &, Settings const &, io::Context &);

  Manager(Manager const &) = delete;

  void operator()(Event<Timer> const &);

 protected:
  // io::net::tcp::Listener::Handler
  void operator()(io::net::tcp::Connection::Factory &) override;
  void operator()(io::net::tcp::Connection::Factory &, io::NetworkAddress const &) override;

  // Session::Handler
  void operator()(Session::Disconnected const &) override;
  void operator()(metrics::Writer &) override;

  void remove_zombies();

 private:
  Handler &handler_;
  Shared shared_;
  std::unique_ptr<io::net::tcp::Listener> listener_;
  uint64_t next_session_id_ = {};
  utils::unordered_map<uint64_t, std::unique_ptr<Session>> sessions_;
  std::chrono::nanoseconds next_cleanup_ = {};
  utils::unordered_set<uint64_t> zombies_;
};

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
