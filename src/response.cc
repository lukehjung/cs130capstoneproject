#include "response.h"

namespace Miscellaneous {
  // Strings used in forming an HTTP Response
  const char separator[] = { ':', ' ' };
  const char crlf[] = { '\r', '\n' };
} // namespace MiscString

namespace Status {
  // Status lines for every status code in HTTP/1.0
  const std::string okay = "HTTP/1.0 200 OK\r\n";
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

 // Gets status line for a given status code
  std::string ToString(Response::ResponseCode status) {
    switch (status) {
      case Response::okay:
        return (okay);
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
} // namespace Status

namespace DefaultResponse {
  // Default message bodies for every status code in HTTP/1.0
  const char okay[] =
    "";
  const char created[] =
    "<html>"
    "<head><title>Created</title></head>"
    "<body><h1>201 Created</h1></body>"
    "</html>";
  const char accepted[] =
    "<html>"
    "<head><title>Accepted</title></head>"
    "<body><h1>202 Accepted</h1></body>"
    "</html>";
  const char no_content[] =
    "<html>"
    "<head><title>No Content</title></head>"
    "<body><h1>204 Content</h1></body>"
    "</html>";
  const char moved_permanently[] =
    "<html>"
    "<head><title>Moved Permanently</title></head>"
    "<body><h1>301 Moved Permanently</h1></body>"
    "</html>";
  const char moved_temporarily[] =
    "<html>"
    "<head><title>Moved Temporarily</title></head>"
    "<body><h1>302 Moved Temporarily</h1></body>"
    "</html>";
  const char not_modified[] =
    "<html>"
    "<head><title>Not Modified</title></head>"
    "<body><h1>304 Not Modified</h1></body>"
    "</html>";
  const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
  const char unauthorized[] =
    "<html>"
    "<head><title>Unauthorized</title></head>"
    "<body><h1>401 Unauthorized</h1></body>"
    "</html>";
  const char forbidden[] =
    "<html>"
    "<head><title>Forbidden</title></head>"
    "<body><h1>403 Forbidden</h1></body>"
    "</html>";
  const char not_found[] =
    "<html>"
    "<head><title>Not Found</title></head>"
    "<body><h1>404 Not Found</h1></body>"
    "</html>";
  const char internal_server_error[] =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
  const char not_implemented[] =
    "<html>"
    "<head><title>Not Implemented</title></head>"
    "<body><h1>501 Not Implemented</h1></body>"
    "</html>";
  const char bad_gateway[] =
    "<html>"
    "<head><title>Bad Gateway</title></head>"
    "<body><h1>502 Bad Gateway</h1></body>"
    "</html>";
  const char service_unavailable[] =
    "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";

  // Gets default message body for a given status code
  std::string ToHtml(Response::ResponseCode status) {
    switch (status) {
      case Response::okay:
        return okay;
      case Response::created:
        return created;
      case Response::accepted:
        return accepted;
      case Response::no_content:
        return no_content;
      case Response::moved_permanently:
        return moved_permanently;
      case Response::moved_temporarily:
        return moved_temporarily;
      case Response::not_modified:
        return not_modified;
      case Response::bad_request:
        return bad_request;
      case Response::unauthorized:
        return unauthorized;
      case Response::forbidden:
        return forbidden;
      case Response::not_found:
        return not_found;
      case Response::internal_server_error:
        return internal_server_error;
      case Response::not_implemented:
        return not_implemented;
      case Response::bad_gateway:
        return bad_gateway;
      case Response::service_unavailable:
        return service_unavailable;
      default:
        return internal_server_error;
    }
  }
} // namespace DefaultResponse


// Default response
Response Response::DefaultResponse(Response::ResponseCode status) {
  Response defaultResponse;
  defaultResponse.status = status;
  defaultResponse.content = DefaultResponse::ToHtml(status);
  defaultResponse.headers.resize(2);
  defaultResponse.headers[0].first = "Content-Length";
  defaultResponse.headers[0].second = std::to_string(r.content.size());
  defaultResponse.headers[1].first = "Content-Type";
  defaultResponse.headers[1].second = "text/html";
  return r;
}

// text/plain response
Response Response::PlainTextResponse(std::string&& text) {
  Response plainText;
  plainText.status = Response::okay;
  plainText.content = std::move(text);
  plainText.headers.resize(2);
  plainText.headers[0].first = "Content-Length";
  plainText.headers[0].second = std::to_string(r.content.size());
  plainText.headers[1].first = "Content-Type";
  plainText.headers[1].second = "text/plain";
  return r;
}

// text/html response
Response Response::HtmlResponse(std::string&& html) {
  Response html;
  html.status = Response::okay;
  html.content = std::move(html);
  html.headers.resize(2);
  html.headers[0].first = "Content-Length";
  html.headers[0].second = std::to_string(r.content.size());
  html.headers[1].first = "Content-Type";
  html.headers[1].second = "text/html";
  return r;
}

// Response
Response::Headers Response::GetHeaders() const { // Get header
  return headers;
}

void Response::AddHeader(const std::string& name,
                         const std::string& value) { // Add header
  headers.push_back(std::make_pair(name, value));
}

std::string Response::GetBody() const { // Get body
  return content;
}

void Response::SetBody(const std::string& body) { // Set body
  content = body;
}

Response::ResponseCode Response::GetStatus() const { // Get status
  return status;
}

void Response::SetStatus(const ResponseCode code) { // Set status
  status = code;
}

void Response::SetFullResponse(const std::string& response) { // Set full response
  full_response = response;
}

std::string Response::ToString() const { // Put all component to the response string
  if (full_response != "") {
    return full_response;
  }
  std::string check;
  check += Status::ToString(status);
  for (std::size_t i = 0; i < headers.size(); ++i) {
    const Response::Header& h = headers[i];
    check += h.first;
    check += std::string(Miscellaneous::separator, sizeof(MiscString::separator));
    check += h.second;
    check += std::string(Miscellaneous::crlf, sizeof(Miscellaneous::crlf));
  }
  check += Miscellaneous::crlf;
  check += content;
  return check;
}
