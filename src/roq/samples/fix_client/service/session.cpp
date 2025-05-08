/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/fix_client/service/session.hpp"

#include "roq/logging.hpp"

#include "roq/exceptions.hpp"

#include "roq/web/rest/server.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

// === IMPLEMENTATION ===

Session::Session(Handler &handler, uint64_t session_id, io::net::tcp::Connection::Factory &factory, Shared &shared)
    : handler_{handler}, session_id_{session_id}, server_{web::rest::Server::create(*this, factory)}, shared_{shared} {
}

bool Session::ready() const {
  return state_ == State::READY;
}

bool Session::zombie() const {
  return state_ == State::ZOMBIE;
}

void Session::close() {
  (*server_).close();
}

// web::rest::Server::Handler

void Session::operator()(web::rest::Server::Disconnected const &) {
  state_ = State::ZOMBIE;
  auto disconnected = Disconnected{
      .session_id = session_id_,
  };
  handler_(disconnected);
}

void Session::operator()(web::rest::Server::Request const &request) {
  log::info("DEBUG request={}"sv, request);
  auto success = false;
  try {
    auto path = request.path;  // note! url path has already been split
    if (!std::empty(path) && !std::empty(shared_.settings.service.url_prefix) && path[0] == shared_.settings.service.url_prefix) {
      path = path.subspan(1);  // drop prefix
    }
    if (!std::empty(path)) {
      Response response{*server_, request, shared_.encode_buffer};
      route(response, request, path);
    }
    success = true;
  } catch (RuntimeError &e) {
    log::error("Error: {}"sv, e);
  } catch (std::exception &e) {
    log::error("Error: {}"sv, e.what());
  }
  if (!success) {
    close();
  }
}

void Session::operator()(web::rest::Server::Text const &) {
  assert(false);
}

void Session::operator()(web::rest::Server::Binary const &) {
  assert(false);
}

void Session::route(Response &response, web::rest::Server::Request const &request, std::span<std::string_view> const &path) {
  switch (request.method) {
    using enum web::http::Method;
    case GET:
      if (path[0] == "metrics"sv) {
        if (std::size(path) == 1) {
          get_metrics(response, request);
        }
      }
      break;
    case HEAD:
      break;
    case POST:
      break;
    case PUT:
      break;
    case DELETE:
      break;
    case CONNECT:
      break;
    case OPTIONS:
      break;
    case TRACE:
      break;
  }
}

// get

void Session::get_metrics(Response &response, web::rest::Server::Request const &request) {
  if (!std::empty(request.query)) {
    throw RuntimeError{"Unexpected: query keys not supported"sv};
  }
  auto &writer = *shared_.metrics_writer;
  writer.clear();
  handler_(writer);
  auto result = writer.get();
  response(web::http::Status::OK, web::http::ContentType::TEXT_PLAIN, "{}"sv, result);
}

// helpers

void Session::send_result(std::string_view const &message, auto const &id) {
  send_jsonrpc("result"sv, message, id);
}

void Session::send_error(std::string_view const &message, auto const &id) {
  send_jsonrpc("error"sv, message, id);
}

template <typename... Args>
void Session::send_text(fmt::format_string<Args...> const &fmt, Args &&...args) {
  shared_.encode_buffer.clear();
  fmt::format_to(std::back_inserter(shared_.encode_buffer), fmt, std::forward<Args>(args)...);
  roq::log::debug(R"(message="{}")"sv, shared_.encode_buffer);
  (*server_).send_text(shared_.encode_buffer);
}

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
