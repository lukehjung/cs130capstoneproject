#include "server.h"

server::server(boost::asio::io_service &io_service, short port, std::vector<config_block> config_blocks)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    INFO << "READY TO ACCEPT";

    /* construct handlers */
    for(config_block block : config_blocks)
    {
      handlers_tackers[block.prefix] = createHandler(block.prefix, block);
    }

    start_accept();
}

void server::start_accept()
{
    session *new_session = new session(io_service_);
    // this sets the new session class to have the vector of the different paths and aliases
    //new_session->setConfigLocation = configLocation;
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}


void server::handle_accept(session *new_session,
                           const boost::system::error_code &error)
{
    if (!error)
    {
        INFO << "ACCEPT CLIENT CONNECTION SUCCESSFULLY";
        /* Get client Ip address */
        boost::asio::ip::tcp::endpoint remote_ep = new_session->socket_.remote_endpoint();
        boost::asio::ip::address remote_ad = remote_ep.address();
        unsigned short remote_pt = remote_ep.port();
        std::string s = "(" + remote_ad.to_string() + ":" + std::to_string(remote_pt) + ")";
        INFO << "IP ADDRESS: " << remote_ad.to_string();
        /* Set client Ip address and port as logging attribute */
        logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["clientIp"]).set(s);
        new_session->start();
    }
    else
    {
        // client disconnected or other errors
        ERROR << error.message();
        // remove ip address
        logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["clientIp"]).set("");
        INFO << "CLOSE CONNECTION";
        delete new_session;
    }

    start_accept();
}

RequestHandler* server::createHandler(const std::string& location_path, const config_block& block)
{
  if(block.handler_type == "StaticHandler")
  {
    RequestHandler* req_handler = StaticFileHandler::Init(location_path, block.content);
    return req_handler;
  }

  else if (block.handler_type == "EchoHandler")
  {
    RequestHandler* req_handler = EchoHandler::Init(location_path, block.content);
    return req_handler;
  }
}
