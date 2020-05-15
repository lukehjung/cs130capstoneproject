#include "request_handler.h"
#include "response.h"
#include "config_parser.h"
#include "utils.h"

#include <iostream>
#include <list>
#include <iterator>
#include <type_traits>

class StatusHandler : public RequestHandler {
    public:
        StatusHandler() {};
        static RequestHandler* Init(const std::string& location_path, const NginxConfig& config);
        Response handleRequest(const Request& request);
        std::string addRecord(std::string uri, std::string handlerName, Response::StatusCode code);
        Response::StatusCode getStatusCode(std::string handlerName);
        std::string getAllStatus();
        std::string statusToStr(Response::StatusCode code);
    private:
        static std::list<std::string> status; 
};