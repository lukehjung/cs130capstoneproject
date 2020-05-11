#include "error_handler.h"
#include "utils.h"
#include <iostream>

std::unique_ptr<RequestHandler> ErrorHandler::Init() {
    std::unique_ptr<RequestHandler> error_handler(new ErrorHandler());
    return error_handler;
}

Response ErrorHandler::handleRequest(const Request& request) {
    std::string message = "No Content Found! Check request format!";
    return utility.plain_text_response(message, Response::not_found);
    return response;
}