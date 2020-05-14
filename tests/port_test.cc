#include "gtest/gtest.h"
#include "port.h"
#include <string>
#include "iostream"
#include "static_file_handler.h"

class PortTest : public ::testing::Test
{
protected:
    bool ParseString(const std::string config_string)
    {
        std::stringstream config_stream(config_string);
        return parser.Parse(&config_stream, &out_config);
    }
    NginxConfigParser parser;
    NginxConfig out_config;
    port test_port;
};

TEST_F(PortTest, CheckPortNum)
{
    const char *filename = "configtests/my_config";
    bool check = test_port.checkPortNum(filename);
    EXPECT_TRUE(check);
    EXPECT_EQ(8081, test_port.getPortNum());
}

TEST_F(PortTest, CheckEmptyPort)
{
    const char *filename = "configtests/empty_statement";
    bool check = test_port.checkPortNum(filename);
    EXPECT_FALSE(check);
}

TEST_F(PortTest, ValidPort)
{
    EXPECT_TRUE(test_port.isValid(9000));
}

TEST_F(PortTest, InvalidPort)
{
    EXPECT_FALSE(test_port.isValid(0));
}

TEST_F(PortTest, NumericPort)
{
    EXPECT_TRUE(test_port.isNumeric("1234"));
}

TEST_F(PortTest, GetMultiplePortNum)
{
    const char *filename = "configtests/multiple_ports";
    bool check = test_port.checkPortNum(filename);
    EXPECT_EQ(8081, test_port.getPortNum());
}

TEST_F(PortTest, CheckFilePath)
{
    const char *filename = "configtests/portspath";
    bool check = test_port.checkFilePath(filename);
    std::vector<std::string> fileMap = test_port.getFilePath();

    std::string alias1 = fileMap[0];
    std::string alias2 = fileMap[2];

    std::string path1 = fileMap[1];
    std::string path2 = fileMap[3];

    EXPECT_EQ(alias1, "/");
    EXPECT_EQ(alias2, "/images/");
    EXPECT_EQ(path1, "/data/www");
    EXPECT_EQ(path2, "/data");
    EXPECT_TRUE(check);
}

TEST_F(PortTest, CheckNonNumericPort)
{
    const char *filename = "configtests/non_numeric_port";
    bool check = test_port.checkPortNum(filename);
    EXPECT_FALSE(check);
}