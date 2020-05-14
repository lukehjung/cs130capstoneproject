#include "gtest/gtest.h"
#include "echo_handler.h"
#include "session.h"
#include "logging.h"
#include "utils.h"
#include <string>
#include "iostream"

class EchoHandlerTest : public ::testing::Test
{
protected:
    bool ParseString(const std::string config_string)
    {
        std::stringstream config_stream(config_string);
        return parser.Parse(&config_stream, &out_config);
    }
    NginxConfigParser parser;
    NginxConfig out_config;
    EchoHandler echo_test;
};

TEST_F(EchoHandlerTest, GetResponse)
{
    std::string request_string = "GET / HTTP/1.1\r\n";
    std::string response_string = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 14\r\nConnection: close\r\n";
    EXPECT_EQ(response_string, echo_test.getResponse(request_string, true));
}
