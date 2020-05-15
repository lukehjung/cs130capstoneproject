#include "error_handler.h"
#include "status_handler.h"
#include "utils.h"
#include <iostream>

RequestHandler* ErrorHandler::Init(const std::string& location_path, const NginxConfig& config) {
    RequestHandler* error_handler = new ErrorHandler();
    return error_handler;
}

Response ErrorHandler::handleRequest(const Request& request) {
    Utils utility;
    StatusHandler status_handler;
    status_handler.addRecord(request, "ErrorHandler", Response::not_found);
    std::string message = "No Content Found! Check request format!";
    return utility.plain_text_response(message, Response::not_found);
}