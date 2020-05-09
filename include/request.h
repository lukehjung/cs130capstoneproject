#ifndef REQUEST_H
#define REQUEST_H

#include <memory>
#include <string>
#include <unordered_map>

/* HTTP Request
   Usage: [ auto request = Request::Parse(raw_request); ]
*/
class Request {
  public:
    /* Parser results from the Parse method */
    enum Result { good, bad, undefined };

    /* Parses data from the client.
       The return value is:
      		Valid when a complete request has been parsed.
      		Nullptr if the data is invalid
      		Undefined
    */
    static std::unique_ptr<Request> Parse(const std::string& raw_request);

    /* Returns the result of the request Parse function */
    static Result GetParseResult();

    /* Default constructor */
    Request();
    void Reset(); /* Reset the request to fresh state */

    /* Request */
    std::string raw_request() const; /* HTTP REQUEST */
    std::string version() const; /* HTTP VERSION */
    std::string method() const; /* HTTP METHOD */
    std::string path() const; /* return URI or EMPTYSTRING */
    std::string uri() const; /* URI for HTTP REQUEST */
    void SetUri(const std::string& uri);

    /* Headers and Body of http request
       Returns:
            The value for given header
            OR the empty string
    */
    std::string GetHeaderValue(const std::string& name) const;
    using Headers = std::unordered_map<std::string, std::string>;
    Headers headers() const;
    std::string body() const;

    /* Handles the next character of input to the parser */
    Result NextCharHandler(char input);

    /* State used internally by the parser */
    enum {
      _method_start,
      _method,
      _uri,
      _version_h,
      _version_firstT,
      _version_secondT,
      _version_p,
      _version_slash,
      _version_major_start,
      _version_major,
      _version_minor_start,
      _version_minor,
      _newline_1,
      _header_start,
      _header_lws,
      _header_name,
      _space_before_header_value,
      _header_value,
      _newline_2,
      _newline_3,
      _body
    } state;

    enum {
      GET,
      POST,
      PUT,
      DELETE,
      HEAD,
      CONNECT,
      OPTIONS,
      TRACE,
      PATCH
    } Method;

    /* fields required by Common API */

    /* The HTML method (GET, PUT, POST, etc) */
    Method method_;
    /* The resource for the request */
    std::string uri_;
    /* Headers included in the request */
    Headers headers_;
    /* Body of the request */
    std::string body_;

    /* Other fields, as convenient for processing */

    /* Used to track remaining characters to parse */
    unsigned long long remaining;
    /* The entire unparsed request */
    std::string raw_request_;
    /* Path represented by URI */
    std::string path_;
    /* HTTP version the requester is using */
    std::string version_;
};

#endif // REQUEST_H
