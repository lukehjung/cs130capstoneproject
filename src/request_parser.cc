// https://www.boost.org/doc/libs/1_50_0/doc/html/boost_asio/example/http/server/request_parser.hpp
#include "request_parser.h"
#include "request_handler.h"

#include <sstream>
#include <tuple>
#include <boost/algorithm/string.hpp>


RequestParser::Result result = RequestParser::undefined;

/* Default constructor */
RequestParser::RequestParser() : state(_method_start) {}

/* Reset the request */
void RequestParser::Reset() {
   raw_request_ = std::string();
   method_ = std::string();
   uri_ = std::string();
   path_ = std::string();
   version_ = std::string();
   headers_ = Headers();
   body_ = std::string();
   state = _method_start;
}

/* Parses data from the client.
   Returns:
   	<RequestParser::good, begin> when complete request has been parsed
   	<RequestParser::bad, begin> if the data is invalid
   	<RequestParser::undefined, begin> if more data is required
    where begin is how much of the input has been consumed.
*/

// I copied and modified it from boost's http example, to use it, refer to
// https://www.boost.org/doc/libs/1_50_0/doc/html/boost_asio/example/http/server/connection.cpp
std::tuple<RequestParser::Result, std::string::iterator> RequestParser::Parse(Request& req,
      std::string::iterator begin, std::string::iterator end) {
  result = undefined;

  while (begin != end) {
    result = NextCharHandler(*begin++);
    // only write to Request if the input is deterministic
    if (result == good || result == bad) {
      req.method_ = getMethod();
      req.uri_ = uri();
      req.version_ = version();
      req.headers_ = getHeaderMap();
      req.body_ = body();
      Reset();
      return std::make_tuple(result, begin);
    }
  }
  /* Undefined request */
  return std::make_tuple(result, begin);;
}

/* Returns the result of the request Parse function */
RequestParser::Result RequestParser::GetParseResult() {
  return result;
}

/* Request */
std::string RequestParser::raw_request() const { /* HTTP REQUEST */
  return raw_request_;
}

std::string RequestParser::version() const { /* HTTP VERSION */
  return version_;
}

std::string RequestParser::method() const { /* HTTP METHOD */
  return method_;
}

std::string RequestParser::path() const { /* PATH - URI OF EMPTY STRING */
  return path_;
}

std::string RequestParser::uri() const { /* HTTP RESOURCE IDENTIFIER */
  return uri_;
}

void RequestParser::SetUri(const std::string& uri) {
  uri_ = uri;
  size_t uri_pos = raw_request_.find(" ") + 1;
  size_t uri_end = raw_request_.find(" ", uri_pos);
  size_t uri_len = uri_end - uri_pos;
  raw_request_.replace(uri_pos, uri_len, uri);
}

/* Header and Body of HTTP REQUEST */
/* Returns:
      The value for the given header
      OR the empty string
*/
std::string RequestParser::GetHeaderValue(const std::string& name) const {
  for (std::size_t i = 0; i < headers_.size(); i++) {
    if (headers_[i].first == name) {
      return headers_[i].second;
    }
  }
  return std::string();
}

using Headers = std::vector<std::pair<std::string, std::string> >;
Headers RequestParser::headers() const { /* header */
  return headers_;
}

std::string RequestParser::body() const { /* body */
  return body_;
}

/* HELPER FUNCTIONS FOR PARSER */
/* CHECK CHARACTER */
bool RequestParser::is_char(int c) {
  return c >= 0 && c <= 127;
}

/* CHECK CONTROL CHARACTER */
bool RequestParser::is_control_char(int c) {
  return (c >= 0 && c <= 31) || (c == 127);
}

/* CHECK SPECIAL CHARACTER */
bool RequestParser::is_special_char(int c) {
  switch (c) {
  case '(': case ')': case '<': case '>': case '@':
  case ',': case ';': case ':': case '\\': case '"':
  case '/': case '[': case ']': case '?': case '=':
  case '{': case '}': case ' ': case '\t':
    return true;
  default:
    return false;
  }
}

/* CHECK DIGIT */
bool RequestParser::is_digit(int c) {
    return c >= '0' && c <= '9';
}

/* HTTP request reference
   https://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html
*/
/* CHECK NEXT CHARACTER INPUT FOR PARSER */
RequestParser::Result RequestParser::NextCharHandler(char input) {
  raw_request_ += input;
  switch (state) {
    case _method_start:
      if (!is_char(input) || is_control_char(input) || is_special_char(input)) {
        return bad;
      }
      else {
        /* update the state */
        state = _method;
        method_.push_back(input);
        return undefined;
      }
    case _method:
      if (input == ' ') {
        /* update the state */
        state = _uri;
        return undefined;
      }
      else if (!is_char(input) || is_control_char(input) || is_special_char(input)) {
        return bad;
      }
      else {
        method_.push_back(input);
        return undefined;
      }
    case _uri:
      if (input == ' ') {
        /* update the state */
        state = _version_h;
        /* Infer the path from URI */
        path_.reserve(uri_.size());
        for (std::size_t i = 0; i < uri_.size(); ++i) {
          if (uri_[i] == '%') {
            if (i + 3 <= uri_.size()) {
              int value = 0;
              std::istringstream is(uri_.substr(i + 1, 2));
              if (is >> std::hex >> value) {
                path_ += static_cast<char>(value);
                i += 2;
              }
              else {
                path_ = std::string();
                return undefined;
              }
            }
            else {
              path_ = std::string();
              return undefined;
            }
          }
          else if (uri_[i] == '+') {
            path_ += ' ';
          }
          else {
            path_ += uri_[i];
          }
        }

        /* Make sure that path does not
           go backwards in the directory */
        if (path_.find("..") != std::string::npos) path_ = std::string();
        /* If path ends in slash (it is a directory)
           then add "index.html" */
        else if (path_[path_.size() - 1] == '/') path_ += "index.html";

        return undefined;
      }
      else if (is_control_char(input)) {
        return bad;
      }
      else {
        uri_.push_back(input);
        return undefined;
      }
    case _version_h:
      if (input == 'H') {
        /* update the state */
        state = _version_firstT;
        version_.push_back('H');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_firstT:
      if (input == 'T') {
        /* update the state */
        state = _version_secondT;
        version_.push_back('T');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_secondT:
      if (input == 'T') {
        /* update the state */
        state = _version_p;
        version_.push_back('T');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_p:
      if (input == 'P') {
        /* update the state */
        state = _version_slash;
        version_.push_back('P');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_slash:
      if (input == '/') {
        /* update the state */
        state = _version_major_start;
        version_.push_back('/');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_major_start:
      if (is_digit(input)) {
        /* update the state */
        state = _version_major;
        version_.push_back(input);
        return undefined;
      }
      else {
        return bad;
      }
    case _version_major:
      if (input == '.') {
        /* update the state */
        state = _version_minor_start;
        version_.push_back('.');
        return undefined;
      }
      else if (is_digit(input)) {
        version_.push_back(input);
        return undefined;
      }
      else {
        return bad;
      }
    case _version_minor_start:
      if (is_digit(input)) {
        /* update the state */
        state = _version_minor;
        version_.push_back(input);
        return undefined;
      }
      else {
        return bad;
      }
    case _version_minor:
      if (input == '\r') {
        /* update the state */
        state = _newline_1;
        return undefined;
      }
      else if (is_digit(input)) {
        version_.push_back(input);
        return undefined;
      }
      else {
        return bad;
      }
    case _newline_1:
      if (input == '\n') {
        /* update the state */
        state = _header_start;
        return undefined;
      }
      else {
        return bad;
      }
    case _header_start:
      if (input == '\r') {
        /* update the state */
        state = _newline_3;
        return undefined;
      }
      else if (!headers_.empty() && (input == ' ' || input == '\t')) {
        state = _header_lws;
        return undefined;
      }
      else if (!is_char(input) || is_control_char(input) || is_special_char(input)) {
        return bad;
      }
      else {
        headers_.push_back(std::pair<std::string, std::string>());
        headers_.back().first.push_back(input);
        /* update the state */
        state = _header_name;
        return undefined;
      }
    case _header_lws:
      if (input == '\r') {
        /* update the state */
        state = _newline_2;
        return undefined;
      }
      else if (input == ' ' || input == '\t') {
        return undefined;
      }
      else if (is_control_char(input)) {
        return bad;
      }
      else {
        /* update the state */
        state = _header_value;
        headers_.back().second.push_back(input);
        return undefined;
      }
    case _header_name:
      if (input == ':') {
        /* update the state */
        state = _space_before_header_value;
        return undefined;
      }
      else if (!is_char(input) || is_control_char(input) || is_special_char(input)) {
        return bad;
      }
      else {
        headers_.back().first.push_back(input);
        return undefined;
      }
    case _space_before_header_value:
      if (input == ' ') {
        /* update the state */
        state = _header_value;
        return undefined;
      }
      else {
        return bad;
      }
    case _header_value:
      if (input == '\r') {
        /* update the state */
        state = _newline_2;
        return undefined;
      }
      else if (is_control_char(input)) {
        return bad;
      }
      else {
        headers_.back().second.push_back(input);
        return undefined;
      }
    case _newline_2:
      if (input == '\n') {
        /* update the state */
        state = _header_start;
        return undefined;
      }
      else {
        return bad;
      }
    case _newline_3:
      if (input == '\n') {
        try {
          remaining = std::stoull(GetHeaderValue("Content-Length"));
          if (remaining > 0) {
            state = _body;
            return undefined;
          }
          else {
            return good;
          }
        }
        catch (...) {
          return good;
        }
      }
      else {
        return bad;
      }
    case _body:
      body_.push_back(input);
      remaining--;
      if (remaining > 0) {
        return undefined;
      }
      else {
        return good;
      }
    default:
      return bad;
  }
}

Request::Method RequestParser::getMethod() {
  std::string method = boost::to_upper_copy<std::string>(method_);;
  Request::Method result;

  if (method == "GET") {
    result = Request::GET;
  } else if (method == "POST") {
    result = Request::POST;
  } else if (method == "PUT") {
    result = Request::PUT;
  } else if (method == "DELETE") {
    result = Request::DELETE;
  } else if (method == "HEAD") {
    result = Request::HEAD;
  } else if (method == "CONNECT") {
    result = Request::CONNECT;
  } else if (method == "OPTIONS") {
    result = Request::OPTIONS;
  } else if (method == "TRACE") {
    result = Request::TRACE;
  } else {
    result = Request::PATCH;
  }
  return result;
}

std::map<std::string, std::string> RequestParser::getHeaderMap() {
  std::map<std::string, std::string> headers_map((headers_.begin()), headers_.end());
  return headers_map;
}

void RequestParser::reset(Request& request_)
{
  request_.method_ = Request::GET;
  request_.uri_ = std::string();
  request_.headers_.clear();
  request_.body_ = std::string();
  request_.version_ = std::string();
}
