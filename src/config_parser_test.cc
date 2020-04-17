#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserStringTest : public ::testing::Test {
  protected:
    bool ParseString(const std::string config_string) {
      std::stringstream config_stream(config_string);
      return parser.Parse(&config_stream, &out_config);
    }

    NginxConfigParser parser;
    NginxConfig out_config;
};

TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);
  // add failure flag
  bool failure = parser.Parse("non_existent_config", &out_config);

  EXPECT_TRUE(success);
  // new adding
  EXPECT_FALSE(failure);
}

TEST(NginxConfigStatementTest, ToString) {
  NginxConfigStatement statement;

  statement.tokens.push_back("hello");
  statement.tokens.push_back("world");

  EXPECT_EQ("hello world;\n", statement.ToString(0));
}

// Testing SimpleStatementConfig
TEST_F(NginxConfigParserStringTest, SimpleStatementConfig) {
  ASSERT_TRUE(ParseString("hello world;"));
  ASSERT_EQ(1, out_config.statements.size()) << "Config has only one statement!";
  EXPECT_EQ("hello", out_config.statements[0]->tokens[0]) << "hello is the first token!";
}

// Testing SimpleInvalidStatementConfig
TEST_F(NginxConfigParserStringTest, SimpleInvalidStatementConfig) {
  EXPECT_FALSE(ParseString("hello world")) << "Config missing semicolon!";
  EXPECT_FALSE(ParseString("hello world;;")) << "Config with repeated semicolon!";
  EXPECT_FALSE(ParseString("hello 'world;")) << "Config missing closed single quote!";
  EXPECT_FALSE(ParseString("hello \"world;")) << "Config missing close double quote!";
  EXPECT_FALSE(ParseString("hello 'world\";")) << "Config with improper matching quote!";
  EXPECT_FALSE(ParseString("")) << "Empty string config!";
  EXPECT_FALSE(ParseString(" ")) << "White space config!";
  EXPECT_FALSE(ParseString("{")) << "Config with only opening curly brace!";
  EXPECT_FALSE(ParseString("}")) << "Config with only closing curly brace!";
  EXPECT_FALSE(ParseString("{}")) << "Config with matching curly braces but no child block statements!";
}

// Testing MultipleStatementConfig
TEST_F(NginxConfigParserStringTest, MultipleStatementConfig) {
  ASSERT_TRUE(ParseString("hello world { hello world; } helloworld;"));
  ASSERT_EQ(2, out_config.statements.size()) << "Config with two statements: one with child block and normal one!";
  EXPECT_EQ("hello world {\n hello world;\n}\n", out_config.statements[0]->ToString(0));
}

// Testing CurlyConfigs
TEST_F(NginxConfigParserStringTest, CurlyConfigs) {
  EXPECT_TRUE(ParseString("hello wold { hello world; hello world; }"));
  EXPECT_EQ(1, out_config.statements.size());
  EXPECT_EQ(2, out_config.statements[0]->child_block->statements.size());
}

// Testing UnbalancedCurlyConfigs
TEST_F(NginxConfigParserStringTest, UnbalancedCurlyConfigs) {
  EXPECT_FALSE(ParseString("hello world { hello world; ")) << "Missing closing curly brace!";
  EXPECT_FALSE(ParseString("hello world hello world; }")) << "Missing opening curly brace!";
  EXPECT_FALSE(ParseString("hello world { hello world { hello world; }"))
  EXPECT_FALSE(ParseString("hello world hello world; } } }"));
}

// Testing EmbeddedCurlyConfigs
TEST_F(NginxConfigParserStringTest, EmbeddedCurlyConfigs) {
  ASSERT_TRUE(ParseString("hello world { hello world { hello world; } }"));
  ASSERT_EQ(1, out_config.statements.size());
  EXPECT_EQ(1, out_config.statements[0]->child_block->statements.size());
  EXPECT_EQ(1, out_config.statements[0]->child_block->statements[0]->child_block->statements.size());
}

// Testing EmptyChildBlock
TEST_F(NginxConfigParserStringTest, EmptyChildBlock) {
  EXPECT_TRUE(ParseString("hello world {}")) << "Valid empty child block!";
  ASSERT_EQ(1, out_config.statements.size());
  EXPECT_EQ(0, out_config.statements[0]->child_block->statements[0].size());
}

// Testing InnerStatementsConfig
TEST_F(NginxConfigParserStringTest, InnerStatementsConfig) {
  ASSERT_TRUE(ParseString("hello world { hello world; hello world; } helloworld;"));
  ASSERT_EQ(2, out_config.statements.size());
  ASSERT_EQ(2, out_config.statements[0]->child_block->statements.size()) << "Child block has two statements!";
  EXPECT_EQ("world hello;\n", out_config.statements[0]->child_block->statements[1]->ToString(0));
  EXPECT_EQ("hello world;\n", out_config.statements[0]->child_block->statements[0]->ToString(0));
}
