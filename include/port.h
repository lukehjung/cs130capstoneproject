// refactoring for individual testing
#include "config_parser.h"
#include <fstream>
#include <algorithm>
#include <queue>
#include <string>
#include <vector>

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

private:
    int portNum;
    std::vector<std::string> fileMap;
};
