#include "gtest/gtest.h"
#include "port.h"

class PortTest : public ::testing::Test {
  protected:
    bool ParseString(const std::string config_string) {
      std::stringstream config_stream(config_string);
      return parser.Parse(&config_stream, &out_config);
    }
    NginxConfigParser parser;
    NginxConfig out_config;
    port test_port;
};

TEST_F(PortTest, CheckPortNum) {
  const char* filename = "my_config";
  bool check = test_port.checkPortNum(filename);
  EXPECT_TRUE(check);
}

TEST_F(PortTest, CheckEmptyPort) {
  const char* filename = "empty_statement";
  bool check = test_port.checkPortNum(filename);
  EXPECT_FALSE(check);
}

TEST_F(PortTest, ValidPort) {
  EXPECT_TRUE(test_port.isValid(9000));
}

TEST_F(PortTest, InvalidPort) {
  EXPECT_FALSE(test_port.isValid(0));
}

TEST_F(PortTest, NumericPort) {
  EXPECT_TRUE(test_port.isNumeric("1234"));
}

TEST_F(PortTest, GetPortNum) {
  const char* filename = "my_config";
  bool check = test_port.checkPortNum(filename);
  EXPECT_EQ(8000, test_port.getPortNum());
}

TEST_F(PortTest, GetMultiplePortNum) {
  const char* filename = "multiple_ports";
  bool check = test_port.checkPortNum(filename);
  EXPECT_EQ(8081, test_port.getPortNum());
}
