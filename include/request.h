#ifndef REQUEST_H
#define REQUEST_H

#include <memory>
#include <string>
#include <vector>

// HTTP Request
// Usage: [ auto request = Request::Parse(raw_request); ]

class Request {
  public:
    // Parser results from the Parse method
    enum Result { good, bad, undefined };

    // Parses data from the client.
    // The return value is:
    // 		Valid when a complete request has been parsed.
    // 		Nullptr if the data is invalid
    //		Undefined
    static std::unique_ptr<Request> Parse(const std::string& raw_request);

    // Returns the result of the request Parse function
    static Result GetParseResult();

    // Default constructor
    Request();
    void Reset(); // Reset the request to fresh state

    // Request
    std::string raw_request() const; // http request
    std::string version() const; // http request version
    std::string method() const; // method for http request
    std::string path() const; // return URI or emptystring
    std::string uri() const; // resource identifier for http request
    void SetUri(const std::string& uri);

    // Headers and Body of http request
    // Returns the value for given header or empty string
    std::string GetHeaderValue(const std::string& name) const;
    using Headers = std::vector<std::pair<std::string, std::string> >;
    Headers headers() const;
    std::string body() const;

  protected:
    // Handles the next character of input to the parser
    Result Consume(char input);

  private:
    // State used internally by the parser
    enum {
      _method_start,
      _method,
      _uri,
      _http_version_h,
      _http_version_t_1,
      _http_version_t_2,
      _http_version_p,
      _http_version_slash,
      _http_version_major_start,
      _http_version_major,
      _http_version_minor_start,
      _http_version_minor,
      _expecting_newline_1,
      _header_line_start,
      _header_lws,
      _header_name,
      _space_before_header_value,
      _header_value,
      _expecting_newline_2,
      _expecting_newline_3,
      _body
    } state;

    // Used to track remaining characters to parse
    unsigned long long remaining;
    // The entire unparsed request
    std::string raw_request_;
    // What is to be performed
    std::string method_;
    // The resource for the request
    std::string uri_;
    // Path represented by URI
    std::string path_;
    // HTTP version the requester is using
    std::string version_;
    // Headers included in the request
    Headers headers_;
    // Body of the request
    std::string body_;
};

#endif // REQUEST_H
