#include "request_handler.h"
#include "config_parser.h"
#include "utils.h"
#include "session.h"

#include <boost/algorithm/string/trim.hpp>

#include <iostream>
#include <sstream> // for std::cout
#include <string>
#include <regex>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class ProxyHandler : public RequestHandler
{
  public:
    static RequestHandler* Init(const std::string& location_path, const NginxConfig& config);
    Response handleRequest(const Request& request);
    void setLocation(std::string location_path, std::string config_root);

  private:
    // path that ProxyHandler responds to
    std::string serve_addr;
    // address that handler is acting as reverse proxy for
    std::string proxy_addr;
};