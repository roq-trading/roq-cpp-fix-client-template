/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/web/rest/server.hpp"

namespace roq {
namespace samples {
namespace fix_client {
namespace service {

struct Response final {
  Response(web::rest::Server &, web::rest::Server::Request const &, std::string &encode_buffer);

  template <typename... Args>
  inline void operator()(web::http::Status status, web::http::ContentType content_type, fmt::format_string<Args...> const &fmt, Args &&...args) {
    encode_buffer_.clear();
    fmt::format_to(std::back_inserter(encode_buffer_), fmt, std::forward<Args>(args)...);
    send(status, content_type, encode_buffer_);
  }

 protected:
  void send(web::http::Status, web::http::ContentType, std::string_view const &body);

 private:
  web::rest::Server &server_;
  web::rest::Server::Request const &request_;
  std::string &encode_buffer_;
};

}  // namespace service
}  // namespace fix_client
}  // namespace samples
}  // namespace roq
