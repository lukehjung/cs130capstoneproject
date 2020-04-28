#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>

/* HTTP response
   Usage:
     [ Response r;
       r.SetStatus(RESPONSE_200);
       r.SetBody(...);
       return r.ToString(); ]
*/

class Response {
  public:
    typedef std::pair<std::string, std::string> Header;

    /* HTTP response codes */
    enum ResponseCode {
      ok = 200,
      created = 201,
      accepted = 202,
      no_content = 204,
      moved_permanently = 301,
      moved_temporarily = 302,
      not_modified = 304,
      bad_request = 400,
      unauthorized = 401,
      forbidden = 403,
      not_found = 404,
      internal_server_error = 500,
      not_implemented = 501,
      bad_gateway = 502,
      service_unavailable = 503
    };

    /* Status of http response */
    void SetStatus(const ResponseCode response_code);
    ResponseCode GetStatus() const;

    /* Header and Body of http response */
    using Headers = std::vector<std::pair<std::string, std::string>>;
    Headers GetHeaders() const;
    void AddHeader(const std::string& name, const std::string& value);
    void SetBody(const std::string& body);
    std::string GetBody() const;

    /* Response */
    static Response DefaultResponse(ResponseCode status);
    static Response PlainTextResponse(std::string&& text);
    static Response HtmlResponse(std::string&& html);
    void SetFullResponse(const std::string& response);

    /* ToString */
    std::string ToString() const;

  private:
    /* Status code of http response */
    ResponseCode status;
    /* Vector of headers for http response */
    std::vector<std::pair<std::string, std::string> > headers;
    /* Body of http response */
    std::string content;
    /* Header + Body of http response */
    std::string full_response;
};

namespace DefaultResponse {
  /* Gets default message body for a given status code */
  std::string ToHtml(Response::ResponseCode status);
} /* namespace DefaultResponse */

#endif // RESPONSE_H
