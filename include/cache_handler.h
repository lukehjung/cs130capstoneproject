#pragma once
#include <iostream>
#include <string>
#include "request_handler.h"
#include "request.h"
#include "response.h"
#include "config_parser.h"
#include "proxy_handler.h"


class CacheHandler : public RequestHandler {
    public:
        CacheHandler() {};
        static RequestHandler* Init(const std::string& location_path, const NginxConfig& config);
        Response handleRequest(const Request& request);
        std::string getUrl(std::string uri);
};