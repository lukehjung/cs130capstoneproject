#include "echo_handler.h"
#include "utils.h"
#include "logging.h"

std::unique_ptr<RequestHandler> EchoHandler::Init() {
    std::unique_ptr<RequestHandler> echo_handler(new EchoHandler());
    return echo_handler;
}

Response EchoHandler::handleRequest(const Request& request) {
    // assume the body is exactly the message we will echo
    // assume the request is valid
    return utility.plain_text_response(request.body_, Response::ok);
}


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