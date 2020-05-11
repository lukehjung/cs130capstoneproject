#include "request_handler.h"
#include "response.h"

#include <iostream>
#include <list>
#include <iterator>

class StatusHandler : public RequestHandler {
    public:
        StatusHandler() {};
        std::unique_ptr<RequestHandler> Init();
        Response handleRequest(const Request& request);
        std::string addRecord(std::string handlerName);
        Response::StatusCode getStatusCode(std::string handlerName);
        std::string getAllStatus();
    private:
        static list <std::string> status; 
}