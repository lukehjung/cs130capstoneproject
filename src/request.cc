#include "request.h"
#include <memory>
#include <sstream>
#include <thread>

// Local variables per thread
thread_local Request::Result result = Request::undefined;
thread_local Request local_request;

// Default constructor
Request::Request() : state(_method_start) {}

void Request::Reset() { // Reset the request
   raw_request_ = std::string();
   method_ = std::string();
   uri_ = std::string();
   path_ = std::string();
   version_ = std::string();
   headers_ = Headers();
   body_ = std::string();
   state = _method_start;
}

// Parses data from the client.
// Returns:
// 	Valid when complete request has been parsed
// 	Nullptr if the data is invalid
//	Undefined
std::unique_ptr<Request> Request::Parse(const std::string& raw_request) {
  result = undefined;
  std::string::const_iterator begin = raw_request.begin();
  std::string::const_iterator end = raw_request.end();
  while (begin != end) {
    result = local_request.Consume(*begin++);
    if (result == good) {
      auto r = std::unique_ptr<Request>(new Request(local_request));
      local_request.Reset();
      return r;
    } else if (result == bad) {
      local_request.Reset();
      return std::unique_ptr<Request>(nullptr);
    }
  }
  // Undefined request
  return std::unique_ptr<Request>(nullptr);
}

// Returns the result of the request Parse function
Request::Result Request::GetParseResult() {
  return result;
}

// Request
std::string Request::raw_request() const { // http request
  return raw_request_;
}

std::string Request::version() const { // http request version
  return version_;
}

std::string Request::method() const { // method for http request
  return method_;
}

std::string Request::path() const { // URI of empty string
  return path_;
}

std::string Request::uri() const { // resource identifier for http request
  return uri_;
}

void Request::SetUri(const std::string& uri) {
  uri_ = uri;
  size_t uri_pos = raw_request_.find(" ") + 1;
  size_t uri_end = raw_request_.find(" ", uri_pos);
  size_t uri_len = uri_end - uri_pos;
  raw_request_.replace(uri_pos, uri_len, uri);
}

// Header and Body of http request
// Returns the value for the given header or the empty string
std::string Request::GetHeaderValue(const std::string& name) const {
  for (std::size_t i = 0; i < headers_.size(); i++) {
    if (headers_[i].first == name) {
      return headers_[i].second;
    }
  }
  return std::string();
}

using Headers = std::vector<std::pair<std::string, std::string> >;
Headers Request::headers() const { // header
  return headers_;
}

std::string Request::body() const { // body
  return body_;
}

// Helper functions for parser
static bool is_char(int c) { // checks if input is character
  return c >= 0 && c <= 127;
}

static bool is_control_char(int c) { // checks if input is control character
  return (c >= 0 && c <= 31) || (c == 127);
}

static bool is_special_char(int c) { // checks if input is special character
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

static bool is_digit(int c) { // checks if input is digit
    return c >= '0' && c <= '9';
}

// HTTP request reference
// https://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html
Request::Result Request::Consume(char input) { // check next char of input for parser
  raw_request_ += input;
  switch (state) {
    case _method_start:
      if (!is_char(input) || is_control_char(input) || is_special_char(input)) {
        return bad;
      }
      else {
        // update the state
        state = _method;
        method_.push_back(input);
        return undefined;
      }
    case _method:
      if (input == ' ') {
        // update the state
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
        // update the state
        state = _version_h;
        // Infer the path from URI
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

        // Make sure that path does not go backwards in the directory
        if (path_.find("..") != std::string::npos) path_ = std::string();
        // If path ends in slash (it is a directory) then add "index.html"
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
        // update the state
        state = _version_firstT;
        version_.push_back('H');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_firstT:
      if (input == 'T') {
        // update the state
        state = _version_secondT;
        version_.push_back('T');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_secondT:
      if (input == 'T') {
        // update the state
        state = _version_p;
        version_.push_back('T');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_p:
      if (input == 'P') {
        // update the state
        state = _version_slash;
        version_.push_back('P');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_slash:
      if (input == '/') {
        // update the state
        state = _version_major_start;
        version_.push_back('/');
        return undefined;
      }
      else {
        return bad;
      }
    case _version_major_start:
      if (is_digit(input)) {
        // update the state
        state = _version_major;
        version_.push_back(input);
        return undefined;
      } 
      else {
        return bad;
      }
    case _version_major:
      if (input == '.') {
        // update the state
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
        // update the state
        state = _version_minor;
        version_.push_back(input);
        return undefined;
      }
      else {
        return bad;
      }
    case _version_minor:
      if (input == '\r') {
        // update the state
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
        // update the state
        state = _header_start;
        return undefined;
      }
      else {
        return bad;
      }
    case _header_start:
      if (input == '\r') {
        // update the state
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
        // update the state
        state = _header_name;
        return undefined;
      }
    case _header_lws:
      if (input == '\r') {
        // update the state
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
        // update the state
        state = _header_value;
        headers_.back().second.push_back(input);
        return undefined;
      }
    case _header_name:
      if (input == ':') {
        // update the state
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
        // update the state
        state = _header_value;
        return undefined;
      }
      else {
        return bad;
      }
    case _header_value:
      if (input == '\r') {
        // update the state
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
        // update the state
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
