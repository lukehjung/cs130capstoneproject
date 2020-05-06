#include "gtest/gtest.h"
#include "utils.h"

class UtilsTest : public ::testing::Test
{
  protected:
    Utils utils;
};

TEST_F(UtilsTest, CheckMethod) {
  EXPECT_TRUE(utils.check_method("GET"));
  EXPECT_TRUE(utils.check_method("PUT"));
  EXPECT_TRUE(utils.check_method("POST"));
  EXPECT_TRUE(utils.check_method("HEAD"));
  EXPECT_TRUE(utils.check_method("DELETE"));
  EXPECT_TRUE(utils.check_method("OPTIONS"));
  EXPECT_TRUE(utils.check_method("TRACE"));
  EXPECT_TRUE(utils.check_method("PATCH"));
  EXPECT_TRUE(utils.check_method("CONNECT"));
}

TEST_F(UtilsTest, CheckHeader) {
  std::string header = "http:\\";
  EXPECT_TRUE(utils.check_header(header));
}

TEST_F(UtilsTest, CheckComplete) {
  std::string crlf = "\r\n";
  size_t bytes = 2;
  EXPECT_TRUE(utils.complete(crlf, bytes));
}

TEST_F(UtilsTest, CheckFilterCRLF) {
  std::string str = "hello";
  EXPECT_EQ(utils.filter_CRLF(str), 5);
}

TEST_F(UtilsTest, CheckRequestLine) {
  std::string request = "GET / HTTP/1.1\r\n";
  EXPECT_TRUE(utils.check_requestLine(request));
}

TEST_F(UtilsTest, CheckRequest) {
  std::string request = "GET / HTTP/1.1\r\n\r\n";
  EXPECT_TRUE(utils.check_request(request));
}
