#include "response.h"
#include "request.h"

class ErrorHandler : public RequestHandler {
    public:
        ErrorHandler() {};
        static ErrorHandler Init();
        Response handleRequest(const Request& request);
}