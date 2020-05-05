// #include "echo_handler.h"

// // initializes the handler. Return OK if successful.
// // uri_prefix: value in the config file.
// // config: contents of the child block
// RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix,
//                                          const NginxConfig& config) {
//   // nothing special is needed to initialize the echo handler
//   return RequestHandler::OK;
// }

// // handles request, and generates a response.
// //          => returns response code.
// // indicating success or failure condition.
// // if response code is not 'OK'.
// //          => the response's contents are undefined.
// // to control requests and responses well.
// //          => need to implement request and response classes.
// RequestHandler::Status EchoHandler::HandleRequest(const Request& request,
//                                                   Response* response) {
//   // response with raw_request string as plain text
//   *response = Response::PlainTextResponse(request.raw_request());
//   return RequestHandler::OK;
// }

#include "echo_handler.h"
#include "session.h"
#include "utils.h"
#include "logging.h"

void EchoHandler::handler(session *Session, std::string request, bool isValid) {
    std::string response = getResponse(request, isValid);
    dispatch(Session, response);
}

std::string EchoHandler::dispatch(session *Session, std::string response) {
    Session->send_response(response);
    return response; // for testing
}

std::string EchoHandler::getResponse(std::string request, bool isValid) {
    Utils utility;
    std::string http_response = "";
    std::string content;
    // check whether the echo request is a standard http request or plain text
    if (isValid) { // valid request - echo 
        content = utility.getContent(request);
        content = content.substr(1, content.size()); // remove leading slash
        content = utility.url_decode(content);  
        http_response += utility.format_status("200 OK");
    } else {
        content = request;
        http_response += utility.format_status("400 Bad Request");
    }

    http_response += utility.format_header("Content-type", "text/plain");
    http_response += utility.format_header("Content-length", std::to_string(content.length()));
    http_response += utility.format_header("Connection", "close");
    http_response += utility.format_end();

    http_response += content;

    return http_response;
}