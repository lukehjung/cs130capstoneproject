#include "request_handler.h"
#include "config_parser.h"
#include "utils.h"

class HealthHandler : public RequestHandler
{
public:
  HealthHandler(){};
  static RequestHandler *Init(const std::string &location_path, const NginxConfig &config);
  Response handleRequest(const Request &request);
};