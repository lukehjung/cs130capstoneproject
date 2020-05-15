#include "status_handler.h"

std::list<std::string> StatusHandler::status;

RequestHandler* StatusHandler::Init(const std::string& location_path, const NginxConfig& config) {
    RequestHandler* status_handler = new StatusHandler();
    return status_handler;
}

Response StatusHandler::handleRequest(const Request& request) {
    Utils utility;
    addRecord(request.uri_, "StatusHandler", Response::ok);
    std::string message = getAllStatus();
    return utility.plain_text_response(message, Response::ok);
}

std::string StatusHandler::addRecord(std::string uri, std::string handlerName, Response::StatusCode code) {
    std::string record = "";
    record += "URI: " + uri + "\n";
  
    if (handlerName == "") { // error code
        record += "Handler: None\n";
    } else {
        record += "Handler: " + handlerName + "\n";
    }
    // May have error
    record += "Status Code: " + statusToStr(code) + "\n\n";

    status.push_back(record);
    return record; // for testing
}

Response::StatusCode StatusHandler::getStatusCode(std::string handlerName) {
    // Not include bad request, need further modification
    if (handlerName == "EchoHandler" ||
        handlerName == "StaticHandler" ||
        handlerName == "StatusHandler") {
            return Response::ok;
    } else {
        return Response::not_found;
    }
}

std::string StatusHandler::getAllStatus() {
    std::string allStatus = "";
    allStatus += "Total Requests: " + std::to_string(status.size()) + "\n\n";
    std::list <std::string> :: iterator it;
    for(it = status.begin(); it != status.end(); ++it) {
        allStatus += *it;
    }

    return allStatus;
}

std::string StatusHandler::statusToStr(Response::StatusCode code) {
    if (code == Response::ok) {
        return "200";
    } else if (code == Response::not_found) {
        return "404";
    } else if (code == Response::bad_request) {
        return "400";
    }
    return "";
}


