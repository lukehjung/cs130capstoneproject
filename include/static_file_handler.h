#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto
#include <iostream>

#include <sstream> // for std::cout

#include <string>
#include <regex>
#include <fstream>
#include <iostream>
#include <vector>

// this class that i made is completely different than the one Huy first put up, I just built
// a parser that works from the session.cc file.  I wasn't sure how to implement request handler

class StaticFileHandler
{
public:
    StaticFileHandler() {}
    int configParser(std::string http_body, std::string &response);
    std::string getResponse(std::string http_request, std::vector<std::string> configLocation);
    bool parseAbsoluteRoot(std::string &location, std::vector<std::string> configLocation);
};

// #include "request_handler.h"

// // retrieves a file from the server's file system
// class StaticFileHandler : public RequestHandler
// {
// public:
//     // initializes the handler.
//     // 		=> returns OK if successful.
//     // uri_prefix: value in the config file.
//     // config: contents of the child block.
//     virtual Status Init(const std::string &uri_prefix,
//                         const NginxConfig &config);

//     // handles request, and generates response.
//     // 		=> returns a response code.
//     // indicating success or failure condition.
//     // if response code is not 'OK'.
//     // 		=> response's contents are undefined.
//     // to control requests and responses well.
//     // 		=> need to implement request and response classes.
//     virtual Status HandleRequest(const Request &request,
//                                  Response *response);

// private:
//     std::string path_prefix; // path prefix used by clients to get this handler
//     std::string root;        // root path used for retrieving files
// };

// REGISTER_REQUEST_HANDLER(StaticFileHandler);
