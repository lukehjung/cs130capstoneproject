#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>

/* HTTP Request
   Usage: [ auto request = Request::Parse(raw_request); ]
*/
class Request {
  public:
    enum Method{
      GET,
      POST,
      PUT,
      DELETE,
      HEAD,
      CONNECT,
      OPTIONS,
      TRACE,
      PATCH
    };

    /* fields required by Common API */

    /* The HTML method (GET, PUT, POST, etc) */
    Method method_;
    /* The resource for the request */
    std::string uri_;
    /* Headers included in the request */
    std::map<std::string, std::string> headers_;
    /* Body of the request */
    std::string body_;
    /* HTTP version the requester is using */
    std::string version_;

    /* Other fields, as convenient for processing */
};

#endif // REQUEST_H
