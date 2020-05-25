#include "gtest/gtest.h"
#include "session.h"
#include "server.h"
#include "config_parser.h"
#include "port.h"
#include "request_parser.h"
#include "static_file_handler.h"
#include <string.h>
#include <iostream>

class SessionTest : public NginxConfig, public ::testing::Test
{
protected:
    boost::asio::io_service io_service;
    std::vector<config_block> config_blocks;
    config_block block;
    server *serv = new server(io_service, 8082, config_blocks);
    session *test_session = new session(io_service, serv);
    NginxConfigParser parser;
    NginxConfig conf;
};

TEST_F(SessionTest, HandleWriteError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::not_supported);
    EXPECT_FALSE(test_session->handle_write(ec));
    delete serv;
}

TEST_F(SessionTest, HandleWriteNoError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::success);
    EXPECT_TRUE(test_session->handle_write(ec));
    delete serv;
}


TEST_F(SessionTest, HandleReadError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::not_supported);
    EXPECT_FALSE(test_session->handle_read(ec, 0));
    delete serv;
}


TEST_F(SessionTest, HandleReadNoError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::success);
    EXPECT_TRUE(test_session->handle_read(ec, 0));
    delete serv;
}

TEST_F(SessionTest, StartCorrectly)
{
    EXPECT_TRUE(test_session->start());
    delete serv;
}

TEST_F(SessionTest, TestErrorHandler)
{
    // delete class server instance to make new one below
    delete serv;

    // need to instantiate all the different objects as if we're creating
    // a new server
    port port_work;
    const char *filename = "configtests/realconf"; // full deploy.conf file
    port_work.setConfigBlocks(filename, &conf);
    server *serv = new server(io_service, 8082, port_work.getConfigBlocks());
    session *test_session = new session(io_service, serv);
    std::string request = "GET /nothere HTTP/1.1\r\n\r\n";
    EXPECT_EQ("HTTP/1.1 404 Not Found\r\n", test_session->good_request(request));

    // delete server so doesn't hang
    delete serv;
}
