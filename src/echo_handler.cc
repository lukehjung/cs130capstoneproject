#include "echo_handler.h"

// initializes the handler. Return OK if successful.
// uri_prefix: value in the config file.
// config: contents of the child block
RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix,
                                         const NginxConfig& config) {
  // nothing special is needed to initialize the echo handler
  return RequestHandler::OK;
}

// handles request, and generates a response.
//          => returns response code.
// indicating success or failure condition.
// if response code is not 'OK'.
//          => the response's contents are undefined.
// to control requests and responses well.
//          => need to implement request and response classes.
RequestHandler::Status EchoHandler::HandleRequest(const Request& request,
                                                  Response* response) {
  // response with raw_request string as plain text
  *response = Response::PlainTextResponse(request.raw_request());
  return RequestHandler::OK;
}

