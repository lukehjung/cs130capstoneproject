#pragma once

#include <memory>
#include <string>
#include <map>
#include "request.h"
#include "response.h"

// Request Handlers Interface
class RequestHandler {
  public:
    // All subclasses must implement a static construction method
    //static RequestHandler Init(const NginxConfig& config);  // passes the parsed block

    // All subclasses implement a method to process requests.
    virtual Response handleRequest(const Request& request) = 0;
};
