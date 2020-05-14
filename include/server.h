#pragma once
#include "port.h"
#include "request_handler.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <memory>
#include <map>

using boost::asio::ip::tcp;

class session;

class server
{
public:
    // map prefix to request handlers
    static std::map<std::string, RequestHandler*> handlers_tackers;

    server(boost::asio::io_service &io_service, short port, std::vector<config_block> config_blocks);

    RequestHandler* createHandler(const std::string& location_path, const config_block& block);

private:
    void start_accept();
    void handle_accept(session *new_session,
                       const boost::system::error_code &error);

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;

    // map prefix to request handlers
    //std::unordered_map<std::string, RequestHandler*> handlers_tackers;
};
