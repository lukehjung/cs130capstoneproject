#include "request_handler.h"

// echoes request back to client.
class EchoHandler : public RequestHandler {
  public:
    // initializes the handler, returns OK if successful.
    // uri_prefix: the value in the config file.
    // config: the contents of the child block.
    virtual Status Init(const std::string& uri_prefix,
                        const NginxConfig& config);

    // handles request, and generates a response.
    // 		=> returns response code.
    // indicating success or failure condition.
    // if response code is not 'OK'.
    // 		=> the response's contents are undefined.
    // to control requests and responses well.
    // 		=> need to implement request and response classes.
    virtual Status HandleRequest(const Request& request,
                                       Response* response);
};


REGISTER_REQUEST_HANDLER(EchoHandler);
