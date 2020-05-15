#include "dispatcher.h"
#include "utils.h"

extern Utils utility;

namespace Status {
  /* Status lines for every status code in HTTP/1.0 */
  const std::string ok = "HTTP/1.0 200 OK\r\n";
  const std::string created = "HTTP/1.0 201 Created\r\n";
  const std::string accepted = "HTTP/1.0 202 Accepted\r\n";
  const std::string no_content = "HTTP/1.0 204 No Content\r\n";
  const std::string moved_permanently = "HTTP/1.0 301 Moved Permanently\r\n";
  const std::string moved_temporarily = "HTTP/1.0 302 Moved Temporarily\r\n";
  const std::string not_modified = "HTTP/1.0 304 Not Modified\r\n";
  const std::string bad_request = "HTTP/1.0 400 Bad Request\r\n";
  const std::string unauthorized = "HTTP/1.0 401 Unauthorized\r\n";
  const std::string forbidden = "HTTP/1.0 403 Forbidden\r\n";
  const std::string not_found = "HTTP/1.0 404 Not Found\r\n";
  const std::string internal_server_error = "HTTP/1.0 500 Internal Server Error\r\n";
  const std::string not_implemented = "HTTP/1.0 501 Not Implemented\r\n";
  const std::string bad_gateway = "HTTP/1.0 502 Bad Gateway\r\n";
  const std::string service_unavailable = "HTTP/1.0 503 Service Unavailable\r\n";

  /* Gets status line for a given status code */
  std::string ToString(Response::StatusCode status) {
    switch (status) {
      case Response::ok:
        return (ok);
      case Response::created:
        return (created);
      case Response::accepted:
        return (accepted);
      case Response::no_content:
        return (no_content);
      case Response::moved_permanently:
        return (moved_permanently);
      case Response::moved_temporarily:
        return (moved_temporarily);
      case Response::not_modified:
        return (not_modified);
      case Response::bad_request:
        return (bad_request);
      case Response::unauthorized:
        return (unauthorized);
      case Response::forbidden:
        return (forbidden);
      case Response::not_found:
        return (not_found);
      case Response::internal_server_error:
        return (internal_server_error);
      case Response::not_implemented:
        return (not_implemented);
      case Response::bad_gateway:
        return (bad_gateway);
      case Response::service_unavailable:
        return (service_unavailable);
      default:
        return (internal_server_error);
    }
  }
} /* namespace Status */

dispatcher::dispatcher(session* session) : session_(session) {}

void dispatcher::dispatch(const Response& response)
{
  std::string response_ = Status::ToString(response.code_);

  for(auto const& header : response.headers_)
  {
    response_ += utility.format_header(header.first, header.second);
  }

  response_ += utility.format_end();

  if(response.src_type < 2)
  {
    response_ += response.body_;
    session_->send_response(response_);
  }

  else
  {
    session_->send_response(response_);

    std::size_t size = response.body_.size();
    std::size_t total_write {0};  // bytes successfully witten
    std::vector<char> content(response.body_.begin(), response.body_.end());

    while (total_write != size ) {
        total_write += session_->socket_.write_some(boost::asio::buffer(&content[0]+total_write, size-total_write));
    }
  }

}