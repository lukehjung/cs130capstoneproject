#pragma once
#include "request_handler.h"
#include "config_parser.h"
#include "utils.h"
#include "session.h"
#include "cached_page.h"

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
#include <curl/curl.h>
using boost::asio::ip::tcp;

class ProxyHandler : public RequestHandler
{
  public:
    static RequestHandler* Init(const std::string& location_path, const NginxConfig& config);
    Response handleRequest(const Request& request);
    void setLocation(std::string location_path, std::string config_root);
    std::string parse_html_body(std::string& msg); 
    
  private:
    // path that ProxyHandler responds to
    std::string serve_addr;
    // address that handler is acting as reverse proxy for
    std::string proxy_addr;
    // cached pages, mapping from request urls to pages
    static std::map<std::string, cached_page> cached_pages;
};

std::size_t write_callback(const char* ptr, std::size_t size, std::size_t byte_count, std::string* msg);
std::size_t header_callback(char *buffer, size_t size, size_t nitems, void *content);
Response return_500(); 
