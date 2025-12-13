/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/io/net/tcp/connection.hpp"

#include "roq/web/rest/server.hpp"

#include "roq/samples/fix_client/service/response.hpp"
#include "roq/samples/fix_client/service/shared.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

struct Session final : public web::rest::Server::Handler {
  struct Disconnected final {
    uint64_t session_id = {};
  };
  struct Handler {
    virtual void operator()(Disconnected const &) = 0;
    virtual void operator()(metrics::Writer &) = 0;
  };

  Session(Handler &, uint64_t session_id, io::net::tcp::Connection::Factory &, Shared &);

 protected:
  bool ready() const;
  bool zombie() const;

  void close();

  // web::rest::Server::Handler
  void operator()(web::rest::Server::Disconnected const &) override;
  void operator()(web::rest::Server::Request const &) override;
  void operator()(web::rest::Server::Text const &) override;
  void operator()(web::rest::Server::Binary const &) override;

  // rest

  void route(Response &, web::rest::Server::Request const &, std::span<std::string_view> const &path);

  void get_metrics(Response &, web::rest::Server::Request const &);

  // helpers

  void send_result(std::string_view const &message, auto const &id);
  void send_error(std::string_view const &message, auto const &id);

  template <typename... Args>
  void send_text(fmt::format_string<Args...> const &, Args &&...);

 private:
  Handler &handler_;
  uint64_t const session_id_;
  std::unique_ptr<web::rest::Server> server_;
  Shared &shared_;
  enum class State { WAITING, READY, ZOMBIE } state_ = {};
};

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
