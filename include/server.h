#include "session.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

class server
{
public:
    // Constructor
    /*
        fileMap is just a vector that holds the different aliases and root paths that are given 
        from the config file.  I pass it through the server so that I can put it into session 
        later on 
    */
    server(boost::asio::io_service &io_service, short port, std::vector<std::string> fileMap);

private:
    void start_accept();
    void handle_accept(session *new_session,
                       const boost::system::error_code &error);

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;

    /* this holds the fileMap variable after construction */
    std::vector<std::string> configLocation;
};
