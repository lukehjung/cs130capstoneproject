#include "server.h"
#include <string>

server::server(boost::asio::io_service& io_service, short port)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
  INFO << "READY TO ACCEPT";
  start_accept();
}


void server::start_accept()
{
  session* new_session = new session(io_service_);
  acceptor_.async_accept(new_session->socket(),
      boost::bind(&server::handle_accept, this, new_session,
        boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session,
  const boost::system::error_code& error)
{
  if (!error)
  {
    INFO << "ACCEPT CLIENT CONNECTION SUCCESSFULLY";
    /* Get client Ip address */
    boost::asio::ip::tcp::endpoint remote_ep = new_session->socket_.remote_endpoint();
    boost::asio::ip::address remote_ad = remote_ep.address();
    std::string s = "(" + remote_ad.to_string() + ")";
    /* Set client Ip address as logging attribute */
    boost::log::core::get()->add_global_attribute("ClientIp",attrs::constant<std::string>(s));

    new_session->start();
  }
  else
  {
    // client disconnected or other errors
    ERROR << error.message();
    delete new_session;
  }

  start_accept();
}
