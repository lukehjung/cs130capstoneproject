#include "gtest/gtest.h"
#include "session.h"
#include "server.h"
#include "config_parser.h"

class SessionTest : public NginxConfig, public ::testing::Test
{
protected:
    boost::asio::io_service io_service;
    session *test_session = new session(io_service);

    NginxConfigParser parser;
    NginxConfig config;
};

/* Move to utils_test.cc

TEST_F(SessionTest, CheckMethod) {
  EXPECT_TRUE(test_session->check_method("GET"));
  EXPECT_TRUE(test_session->check_method("PUT"));
  EXPECT_TRUE(test_session->check_method("POST"));
  EXPECT_TRUE(test_session->check_method("HEAD"));
  EXPECT_TRUE(test_session->check_method("DELETE"));
  EXPECT_TRUE(test_session->check_method("OPTIONS"));
  EXPECT_TRUE(test_session->check_method("TRACE"));
  EXPECT_TRUE(test_session->check_method("PATCH"));
  EXPECT_TRUE(test_session->check_method("CONNECT"));
}

TEST_F(SessionTest, CheckComplete) {
  std::string crlf = "\r\n";
  size_t bytes = 2;
  EXPECT_TRUE(test_session->complete(crlf, bytes));
}

TEST_F(SessionTest, CheckFilterCRLF) {
  std::string str = "hello";
  EXPECT_EQ(test_session->filter_CRLF(str), 5);
}

TEST_F(SessionTest, CheckHeader) {
  std::string header = "http:\\";
  EXPECT_TRUE(test_session->check_header(header));
}

TEST_F(SessionTest, CheckRequestLine) {
  std::string request = "GET / HTTP/1.1\r\n";
  EXPECT_TRUE(test_session->check_requestLine(request));
}

TEST_F(SessionTest, CheckRequest) {
  std::string request = "GET / HTTP/1.1\r\n\r\n";
  EXPECT_TRUE(test_session->check_request(request));
}
*/
// TEST_F(SessionTest, GoodGetRequest) {
//     std::string beginning = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
//     std::string standard_request = "GET / HTTP/1.1\r\n";
//     std::vector<std::string> fileMap = {"/", "/data/www", "/images/", "/data"};
//     EXPECT_EQ(test_session->good_request(standard_request, fileMap), beginning);
// }
//
// TEST_F(SessionTest, BadRequest) {
//   std::string beginning = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain";
//   std::string standard_request = "GET HTTP/1.1";
//   EXPECT_EQ(test_session->bad_request(standard_request), beginning + standard_request);
// }

TEST_F(SessionTest, HandleWriteError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::not_supported);
    EXPECT_FALSE(test_session->handle_write(ec));
}

TEST_F(SessionTest, HandleWriteNoError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::success);
    EXPECT_TRUE(test_session->handle_write(ec));
}

TEST_F(SessionTest, HandleReadError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::not_supported);
    EXPECT_FALSE(test_session->handle_read(ec, 0));
}

TEST_F(SessionTest, HandleReadNoError)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::success);
    EXPECT_TRUE(test_session->handle_read(ec, 0));
}

TEST_F(SessionTest, HandleReadString)
{
    EXPECT_TRUE(test_session->long_string_handler("GET / HTTP/1.1\r\n\r\n", 14));
}

TEST_F(SessionTest, StartCorrectly)
{
    EXPECT_TRUE(test_session->start());
}

// TEST_F(SessionTest, GoodRequestforHello)
// {
//     std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
//                             "hello";
//     std::string standard_request = "GET /hello.txt HTTP/1.1\r\n";
//     std::vector<std::string> fileMap = {"/", "/data/www", "/images/", "/data"};
//     EXPECT_EQ(test_session->good_request(standard_request, fileMap), response);
// }
