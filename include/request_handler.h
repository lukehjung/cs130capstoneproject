#include <memory>
#include <string>
#include <map>
#include <config_parser.h>
// request and response headers are already added on Gerrit.
// there could be some changes of those headers and implementations.
#include <request.h>
#include <response.h>

// represents the parent of all request handlers.
class RequestHandler {
  public:
    // return code.
    enum Status { OK = 0, INVALID = 1 };

    // registers a request handler by name.
    static RequestHandler* CreateByName(const char* type);

    // initializes the handler. Returns OK if successful.
    // uri_prefix: the value in config file.
    // config: the contents of child block.
    virtual Status Init(const std::string& uri_prefix,
                        const NginxConfig& config) = 0;
    // handles the request, and generates a response.
    // 		=> returns response code.
    // indicates success of failure condition.
    // if response code is not 'OK'.
    // 		=> the response's contents are undefined.
    // to control requests and responses well.
    // 		=> need implement request and response classes.
    virtual Status HandleRequest(const Request& request,
                                       Response* response) = 0;
};

extern std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders;
template<typename T>
class RequestHandlerRegisterer {
  public:
    RequestHandlerRegisterer(const std::string& type) {
      if(request_handler_builders == nullptr) {
        request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>;
      }
      (*request_handler_builders)[type] = RequestHandlerRegisterer::Create;
    }

    static RequestHandler* Create() {
      return new T;
    }
};

#define REGISTER_REQUEST_HANDLER(ClassName) \
  static RequestHandlerRegisterer<ClassName> ClassName##__registerer(#ClassName)
