#include "request_handler.h"

class ErrorHandler : public RequestHandler {
    public:
        ErrorHandler() {};
        std::unique_ptr<RequestHandler> Init();
        Response handleRequest(const Request& request);
}