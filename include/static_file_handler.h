#include "request_handler.h"
#include "config_parser.h"
#include "utils.h"
#include "session.h"

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

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

class StaticFileHandler : public RequestHandler
{
  public:
    static std::map<std::string, std::string> locationToRoot;

    StaticFileHandler() {}
    static RequestHandler* Init(const std::string& location_path, const NginxConfig& config);
    Response handleRequest(const Request& request);

    void handler(session *Session, std::string request);

    std::string getRequestLine(const Request& request);

    std::string replace_path(const std::string& location_prefix);

    // modified version of sendBinary()
    Response getBinaryContent(std::string filename, int src_type);
    // modified version of getResponse()
    Response formResponse(int src_type, std::string file_path);

    /* Helper functions for parsing */
    int configParser(std::string http_body);
    std::string getResponse(std::string http_request, std::vector<std::string> configLocation);
    bool parseAbsoluteRoot(std::string& location, std::vector<std::string> configLocation);

    /* Temporary Dispatch mechanism */
    void dispatch(session *Session, std::string header, std::vector<char> content);
    void send_binary(session *Session, std::string filename, int src_type);
};
