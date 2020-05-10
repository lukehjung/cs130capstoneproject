#include "config_parser.h"
#include "request_handler.h"

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <iostream>
#include <sstream> // for std::cout
#include <string>
#include <regex>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory> // for smart pointers

#include <boost/bind.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class session;

// this class that i made is completely different than the one Huy first put up, I just built
// a parser that works from the session.cc file.  I wasn't sure how to implement request handler

class StaticFileHandler : public RequestHandler
{
  public:
    StaticFileHandler(std::string dir) : root(dir) {}
    static std::unique_ptr<RequestHandler> Init(const NginxConfig& config);  // passes the parsed block
    Response handleRequest(const Request& request);

    void handler(session *Session, std::string request);

    void set_prefix(std::string prefix);
    std::string get_prefix() const;

    // modified version of sendBinary()
    Response getBinaryContent(std::string filename, int src_type);
    // modified version of getResponse()
    Response formResponse(std::string http_request, std::vector<std::string> configLocation);

    /* Helper functions for parsing */
    int configParser(std::string http_body);
    std::string getResponse(std::string http_request, std::vector<std::string> configLocation);
    bool parseAbsoluteRoot(std::string &location, std::vector<std::string> configLocation);

    /* Temporary Dispatch mechanism */
    void dispatch(session *Session, std::string header, std::vector<char> content);
    void send_binary(session *Session, std::string filename, int src_type);

  private:
    std::string path_prefix; // path prefix used by clients to get this handler
    std::string root;        // root path used for retrieving files
};
