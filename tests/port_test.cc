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
};

TEST_F(PortTest, CheckPortNumTest) {

}
