#include "port.h"

bool port::checkPortNum(const char* file_name)
{
  NginxConfigParser config_parser;

  // Convert input config file into input stream type
  std::ifstream config_file;
  config_file.open(file_name);
  std::istream* input = dynamic_cast<std::istream*>(&config_file);

  // Get port number from the given config file
  std::string last_token;
  std::string token = config_parser.getToken(input);

  std::queue<std::string> port_queue;

  // Run through each token to find port number
  while (token.length() > 0)
  {
    if (last_token == "listen")
    {
      port_queue.push(token);
    }

    last_token = token;
    token = config_parser.getToken(input);
  }

  if (port_queue.empty())
  {
    return false;
  }

  // if there are multiple port numbers, we pick the first one for now
  std::string first_port = port_queue.front();

  // Check non-numeric port number
  if(!isNumeric(first_port) || !isValid(std::stoi(first_port)))
  {
    return false;
  }

  // port number is valid, we accept it
  setPortNum(std::stoi(first_port));
  return true;
}

// Check if port number is in valid range
bool port::isValid(int port_num)
{
  if (port_num < 1 || port_num > 65535)
  {
    return false;
  }

  return true;
}

// Check non-numeric port number
bool port::isNumeric(std::string port_num)
{
  if (!std::all_of(port_num.begin(), port_num.end(), ::isdigit))
  {
    return false;
  }

  return true;
}

void port::setPortNum(int port_num)
{
  portNum = port_num;
}

int port::getPortNum()
{
  return portNum;
}
