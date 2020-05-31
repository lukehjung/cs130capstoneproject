#include "health_handler.h"
#include "status_handler.h"
#include "utils.h"
#include <iostream>

RequestHandler *HealthHandler::Init(const std::string &location_path, const NginxConfig &config)
{
  RequestHandler *health_handler = new HealthHandler();
  return health_handler;
}

Response HealthHandler::handleRequest(const Request &request)
{
  Utils utility;
  StatusHandler status_handler;
  status_handler.addRecord(request.uri_, "HealthHandler", Response::ok);
  std::string message = "OK";
  return utility.plain_text_response(message, Response::ok);
}