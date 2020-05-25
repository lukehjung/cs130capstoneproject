#include "server.h"
#include "logging.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

void handler(const boost::system::error_code &error, int signal_number)

{

    ERROR << "Server close socket";

    exit(1);
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }
        boost::asio::io_service io_service;

        // catch keyboard interrupt

        boost::asio::signal_set signals(io_service, SIGINT);

        // Start an asynchronous wait for one of the signals to occur.

        signals.async_wait(handler);

        port p;
        if (!p.checkPortNum(argv[1]))
        {
            ERROR << "Invalid Port Number";
            return 1;
        }

        /*
        if (!p.checkFilePath(argv[1]))
        {
            ERROR << "Invalid File Path\n";
            return 1;
        }
        */

        // will check validity later
        NginxConfig config;
        p.setConfigBlocks(argv[1], &config);

        INFO << "Start listening on port " << p.getPortNum();
        server s(io_service, p.getPortNum(), p.getConfigBlocks());
        s.start_accept();
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
