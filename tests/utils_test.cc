#include "gtest/gtest.h"
#include "request.h"
#include "utils.h"

class UtilsTest : public ::testing::Test
{
protected:
    Utils utils;
};

TEST_F(UtilsTest, CheckMethod)
{
    EXPECT_EQ(utils.check_method("GET"), Request::GET);
    EXPECT_EQ(utils.check_method("PUT"), Request::PUT);
    EXPECT_EQ(utils.check_method("POST"), Request::POST);
    EXPECT_EQ(utils.check_method("HEAD"), Request::HEAD);
    EXPECT_EQ(utils.check_method("DELETE"), Request::DELETE);
    EXPECT_EQ(utils.check_method("OPTIONS"), Request::OPTIONS);
    EXPECT_EQ(utils.check_method("TRACE"), Request::TRACE);
    EXPECT_EQ(utils.check_method("PATCH"), Request::PATCH);
    EXPECT_EQ(utils.check_method("CONNECT"), Request::CONNECT);
}

TEST_F(UtilsTest, CheckHeader)
{
    std::string header = "http:\\";
    EXPECT_TRUE(utils.check_header(header));
}

TEST_F(UtilsTest, CheckComplete)
{
    std::string crlf = "\r\n";
    size_t bytes = 2;
    EXPECT_TRUE(utils.complete(crlf, bytes));
}

TEST_F(UtilsTest, CheckFilterCRLF)
{
    std::string str = "hello";
    EXPECT_EQ(utils.filter_CRLF(str), 5);
}

TEST_F(UtilsTest, CheckRequestLine)
{
    std::string request = "GET / HTTP/1.1\r\n";
    EXPECT_TRUE(utils.check_requestLine(request));
}

TEST_F(UtilsTest, CheckRequest)
{
    std::string request = "GET / HTTP/1.1\r\n\r\n";
    EXPECT_TRUE(utils.check_request(request));
}

TEST_F(UtilsTest, CheckInvalidRequest)
{
    std::string request = "GET HTTP/1.1\r\n\r\n";
    EXPECT_FALSE(utils.check_request(request));
}

TEST_F(UtilsTest, CheckgetContent)
{
    std::string request = "GET /static_images/test.png HTTP/1.1\r\n\r\n";
    EXPECT_EQ("/static_images/test.png", utils.getContent(request));
}