#include "gtest/gtest.h"
#include "session.h"

class SessionTest : public ::testing::Test {
  public:
    bool Check(std::string method) {
      boost::asio::io_service io_service;
      session s(io_service);
      return s.check_method(method);
    }
};

TEST_F(SessionTest, CheckMethod) {
  EXPECT_TRUE(Check("GET"));
  EXPECT_TRUE(Check("PUT"));
}
