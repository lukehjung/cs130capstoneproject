#include "request.h"
#include <memory>
#include <string>
#include <map>
#include <vector>

class RequestParser {
    public:
    /* Default constructor */
    RequestParser();
    void Reset(); /* Reset the request to fresh state */

    /* Parser results from the Parse method */
    enum Result { good, bad, undefined };

    std::tuple<RequestParser::Result, char*> Parse(Request& req,
      char* begin, char* end);

    /* Returns the result of the request Parse function */
    Result GetParseResult();

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
    using Headers = std::vector<std::pair<std::string, std::string>>;
    Headers headers() const;
    std::string body() const;

    /* Handles the next character of input to the parser */
    Result NextCharHandler(char input);

    // convert method string to Request::Method
    Request::Method getMethod();
    // format header into map
    std::map<std::string,std::string> getHeaderMap();

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

    // enum Method{
    //   GET,
    //   POST,
    //   PUT,
    //   DELETE,
    //   HEAD,
    //   CONNECT,
    //   OPTIONS,
    //   TRACE,
    //   PATCH
    // };

    /* Used to track remaining characters to parse */
    unsigned long long remaining;
    /* The entire unparsed request */
    std::string raw_request_;
    /* Path represented by URI */
    std::string path_;
    /* HTTP version the requester is using */
    std::string version_;
    private:
        std::string method_;
        /* The resource for the request */
        std::string uri_;
        /* Headers included in the request */
        Headers headers_;
        /* Body of the request */
        std::string body_;
        bool is_char(int c);
        bool is_control_char(int c);
        bool is_special_char(int c);
        bool is_digit(int c);
};
