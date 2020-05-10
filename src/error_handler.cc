#include "error_handler.h"
#include <iostream>

static ErrorHandler ErrorHandler::Init() {
    static ErrorHandler error_handler;
    return error_handler;
}

Response ErrorHandler::handleRequest(const Request& request) {
    Response response;
    std::string message = "No Content Found! Check request format!";
    response.code_ = response::not_found;
   
    response.headers_["Content-type"] = "text/plain";
    response.headers_["Content-length"] = std::to_string(message.length());
    response.headers_["Connection"] = "close";
    response.body_ = message;
    return response;
}