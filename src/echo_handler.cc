#include "echo_handler.h"
#include "status_handler.h"
#include "utils.h"

RequestHandler* EchoHandler::Init(const std::string& location_path, const NginxConfig& config) {
    RequestHandler* echo_handler = new EchoHandler();
    return echo_handler;
}

Response EchoHandler::handleRequest(const Request& request) {
    Utils utility;
    StatusHandler status_handler;
    status_handler.addRecord(request.uri_, "EchoHandler", Response::ok);
    std::string body = getEchoBody(request.uri_);
    return utility.plain_text_response(body, Response::ok);
}

std::string EchoHandler::getEchoBody(std::string uri) {
    int pos = uri.find_last_of("/") + 1;
    std::string body = uri.substr(pos, uri.size());
    return body;
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
