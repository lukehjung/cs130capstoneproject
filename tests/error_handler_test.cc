#include "error_handler.h"
#include "gtest/gtest.h"
#include "config_parser.h"

class ErrorHandlerTest : public ::testing::Test
{
protected:
    ErrorHandler error;
};

struct config_block
{
    std::string prefix;
    std::string handler_type;
    NginxConfig content;
};

TEST_F(ErrorHandlerTest, BasicTest)
{
    const std::string location_path = "error";
    const config_block block;
    RequestHandler *req_handler = error.Init(location_path, block.content);

    EXPECT_TRUE(true);
}