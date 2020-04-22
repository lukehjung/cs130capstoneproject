#include "request_handler.h"

// retrieves a file from the server's file system
class StaticFileHandler : public RequestHandler {
  public:
    // initializes the handler.
    // 		=> returns OK if successful.
    // uri_prefix: value in the config file.
    // config: contents of the child block.
    virtual Status Init(const std::string& uri_prefix,
                        const NginxConfig& config);

    // handles request, and generates response.
    // 		=> returns a response code.
    // indicating success or failure condition.
    // if response code is not 'OK'.
    // 		=> response's contents are undefined.
    // to control requests and responses well.
    // 		=> need to implement request and response classes.
    virtual Status HandleRequest(const Request& request,
                                       Response* response);

private:
    std::string path_prefix; // path prefix used by clients to get this handler
    std::string root;        // root path used for retrieving files
};

REGISTER_REQUEST_HANDLER(StaticFileHandler);
