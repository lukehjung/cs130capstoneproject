#pragma once

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/xpressive/xpressive.hpp>      // for regex
#include <boost/algorithm/string/replace.hpp> // for replace_all
#include <boost/range/algorithm/count.hpp>    // string count
#include <boost/thread/thread.hpp>
#include <boost/thread/future.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>


#include "logging.h"
#include "request.h"
#include "server.h"
//#include "request_parser.h"

using boost::asio::ip::tcp;
using boost::xpressive::sregex;

class session
{
  public:
    // Constructor
    session(boost::asio::io_service &io_service, server* server);
    tcp::socket &socket() { return socket_; }
    bool start();

    bool long_string_handler(std::string request, size_t bytes_transferred);
    bool handle_read(const boost::system::error_code &error,
                     size_t bytes_transferred);

    bool handle_write(const boost::system::error_code &error);

    // Helper functions for request parser
    void send_response(std::string response);
    std::string good_request(std::string request);
    std::string bad_request(std::string &body);

    void handler_task(bool found, std::string prefix, boost::promise<Response> &res);

    bool request_start;    // mark the start of the request
    std::string http_body; // store one http request
    /* Store http request */
    Request request_;

    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];

  private:
    /* Used to access request handlers */
    server* server_;
    boost::mutex mutex_;    
};
