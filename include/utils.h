#ifndef UTILS_H_
#define UTILS_H_

#include "response.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/xpressive/xpressive.hpp>      // for regex
#include <boost/algorithm/string/replace.hpp> // for replace_all

using boost::asio::ip::tcp;
using boost::xpressive::sregex;

class Utils {
    public:
        Utils(){}
        bool check_requestLine(std::string request);
        int filter_CRLF(std::string request);
        bool complete(std::string request, size_t bytes_transferred);
        bool check_header(std::string header);
        bool check_method(std::string method);
        bool check_request(std::string request);
        std::string format_status(std::string status);
        std::string format_header(std::string key, std::string value);
        std::string format_end();
        std::string getContent(std::string request);
        std::string url_decode(const std::string& in);
        Response plain_text_response(const std::string& text, Response::StatusCode code);
};

#endif
