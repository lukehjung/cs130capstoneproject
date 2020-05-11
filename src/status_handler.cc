#include "status_handler.h"

std::unique_ptr<RequestHandler> StatusHandler::Init() {
    std::unique_ptr<RequestHandler> status_handler(new StatusHandler());
    return status_handler;
}

Response StatusHandler::handleRequest(const Request& request) {
    std::string message = getAllStatus();
    return utility.plain_text_response(message, Response::OK);
}

std::string StatusHandler::addRecord(const Request& request, std::string handlerName, Response::StatusCode error_) {
    std::string record = "";
    record += "URI: " + request.uri_ + "\n";
    Response::StatusCode code;
    if (handlerName == NULL) { // error code
        code = error_;
        record += "Handler: None\n";
        
    } else {
        code = getStatusCode(handlerName);
        record += "Handler: " + handlerName + "\n";
    }
    record += "Status code: " +
               std:：string(static_cast<std::underlying_type<Response::StatusCode>::type>(code)) + "\n\n";

    status.push_back(record);
    return record; // for testing
}

Response::StatusCode StatusHandler::getStatusCode(std::string handlerName) {
    // Not include bad request, need further modification
    if (handlerName == "EchoHandler" ||
        handlerName == "StaticHandler" ||
        handlerName == "StatusHandler") {
            return Response::OK;
    } else {
        return Response::not_found;
    }
}

std::string StatusHandler::getAllStatus() {
    std::string allStatus = "";
    allStatus += "Total Requests: " + std::to_string(status.size()) + "\n";
    list <std::string> :: iterator it;
    for(it = status.begin(); it != status.end(); ++it) {
        allStatus += *it;
    }

    return allStatus;
}



