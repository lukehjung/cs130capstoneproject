#include "gtest/gtest.h"
#include "webserver_test.h"
#include <sstream>

TEST_F(NginxConfigParserTest, SimpleConfig) {
  std::stringstream config("server {listen 8080;}");
  bool success = parser.Parse(&config, &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, NonexistentConfig) {
  bool success = parser.Parse("hello_world", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, EmptyConfig) {
  std::stringstream config("");
  bool success = parser.Parse(&config, &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, singleBraceConfig) {
  std::stringstream config("foo \"bar\";}");
  bool success = parser.Parse(&config, &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, NestedContextConfig) {
  std::stringstream config("http{ server {listen 8080;}}");
  bool success = parser.Parse(&config, &out_config);
  EXPECT_TRUE(success);
}

