#pragma once
// refactoring for individual testing
#include "config_parser.h"
#include <fstream>
#include <algorithm>
#include <queue>
#include <string>
#include <vector>

struct config_block {
  std::string prefix;
  std::string handler_type;
  NginxConfig content;
};

class port
{
public:
    port() {}
    bool checkPortNum(const char *file_name); //check if portNum esits or valid
    bool isValid(int portNum);
    bool isNumeric(std::string port_num);
    void setPortNum(int port_num);
    int getPortNum();

    bool checkFilePath(const char *file_name);
    void setFilePath(std::string alias, std::string path);
    std::vector<std::string> getFilePath();
    bool setConfigBlocks(const char *file_name, NginxConfig* config);
    std::vector<config_block> getConfigBlocks() const;

private:
    int portNum;
    std::vector<std::string> fileMap;
    std::vector<config_block> config_blocks;
};
