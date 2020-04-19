#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"
#include "signal.h"
#include "boost/bind.hpp"
using ::testing::AtLeast;

class MockServer : public server {
  public:
    boost::asio::io_service io_service;
    MockServer() : http::server::server(io_service, 8080) {}
    MOCK_METHOD0(start_accept, void());
    MOCK_METHOD0(handle_accept, void());
};

TEST(ServerTest, startTest) {
  MockServer mock;
  EXPECT_CALL(mock, start_accept()).Times(AtLeast(1));
  EXPECT_CALL(mock, handle_accept()).Times(AtLeast(0));
  mock.start_accept();
}

/*
class ServerTest : public ::testing::Test {
  protected:
    NginxConfigParser parser;
    NginxConfig out_config;
};

TEST_F(ServerTest, GetPortFromConfigFile) {
  parser.Parse("example_config", &out_config);
  int port = getPort(out_config);
  EXPECT_TRUE(port == 8080)
}
*/
