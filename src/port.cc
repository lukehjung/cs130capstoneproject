#include "port.h"
#include "logging.h"

bool port::checkPortNum(const char *file_name)
{
    NginxConfigParser config_parser;
    // Convert input config file into input stream type
    std::ifstream config_file;
    config_file.open(file_name);
    std::istream *input = dynamic_cast<std::istream *>(&config_file);
    // Get port number from the given config file
    std::string last_token;
    std::string token = config_parser.getToken(input);
    std::queue<std::string> port_queue;
    // Run through each token to find port number
    while (token.length() > 0)
    {
        if (last_token == "port")
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
    if (!isNumeric(first_port) || !isValid(std::stoi(first_port)))
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

bool port::checkFilePath(const char *file_name)
{
    NginxConfigParser config_parser;

    // Convert input config file into input stream type
    std::ifstream config_file;
    config_file.open(file_name);
    std::istream *input = dynamic_cast<std::istream *>(&config_file);
    std::string last_token;
    std::string token = config_parser.getToken(input);
    std::queue<std::string> filePath_Queue;
    // Run through each token to find filepath
    while (token.length() > 0)
    {
        if (last_token == "location")
        {
            filePath_Queue.push(token);
            token = config_parser.getToken(input);
            if (token == "{")
            {
                token = config_parser.getToken(input);
                while (token != ";")
                {
                    filePath_Queue.push(token);
                    token = config_parser.getToken(input);
                }
            }
        }
        last_token = token;
        token = config_parser.getToken(input);
    }
    // do % 3 to make sure the queue has a size of multiple of 3, since we're
    // putting in keywords of the alias, root keyword, and the actual path
    if (filePath_Queue.empty() || (filePath_Queue.size() % 3) != 0)
    {
        return false;
    }
    // go through queue to find the alias and paths, pop 3 times because that's the
    // size of each multiple
    while (filePath_Queue.size() > 0)
    {
        std::string first_path = filePath_Queue.front();
        filePath_Queue.pop();
        filePath_Queue.pop();
        std::string locationPath = filePath_Queue.front();
        filePath_Queue.pop();
        setFilePath(first_path, locationPath);
    }
    return true;
}

/*
    To explain the location server block in Nginx config:
    Consider if this was the block in the nginx config file
    location / {
        root /data/www;
    }
    If we gave the webserve the HTTP request of http://localhost/some/example.html
    we will find the file at  /data/www/some/example.html
    Ref: http://nginx.org/en/docs/beginners_guide.html#conf_structure
*/
void port::setFilePath(std::string alias, std::string path)
{
    fileMap.push_back(alias);
    fileMap.push_back(path);
}
std::vector<std::string> port::getFilePath()
{
    return fileMap;
}

bool port::setConfigBlocks(const char *file_name, NginxConfig* config)
{
  NginxConfigParser config_parser;
  // This will set up the block contents for us to retrieve later
  config_parser.Parse(file_name, config);

  // Convert input config file into input stream type
  std::ifstream config_file;
  config_file.open(file_name);
  std::istream *input = dynamic_cast<std::istream *>(&config_file);
  std::string last_token;
  std::string token = config_parser.getToken(input);

  // Run through each token to get block headers
  while (token.length() > 0)
  {
      if (last_token == "location")
      {
          config_block block;
          block.prefix = token;
          token = config_parser.getToken(input);
          block.handler_type = token;

          if(token == "EchoHandler")
          {
            // echo handler block is finished here
            config_blocks.push_back(block);
            while(token != "}")
              token = config_parser.getToken(input);
          }

          else if(token == "StatusHandler")
          {
            // status handler block is finished here
            config_blocks.push_back(block);
            while(token != "}")
              token = config_parser.getToken(input);
          }

          else if(token == "ErrorHandler")
          {
            // error handler block is finished here
            config_blocks.push_back(block);
            while(token != "}")
              token = config_parser.getToken(input);
          }

          else
            token = config_parser.getToken(input);

          if (token == "{")
          {
              // static handler block is finished here
              config_blocks.push_back(block);
              token = config_parser.getToken(input);
              while (token != ";")
              {
                  token = config_parser.getToken(input);
              }
          }
      }

      last_token = token;
      token = config_parser.getToken(input);
  }

  /* havent checked invalid config format, will do later */


  // Assign each config block content to its corresponding block header
  std::vector<NginxConfig> blocks = config_parser.getConfigBlocksContent();
  for(int i = 0; i < blocks.size(); i++)
  {
    config_blocks[i].content = blocks[i];
  }

  INFO << "DONE";
  return true;
}

std::vector<config_block> port::getConfigBlocks() const
{
  return config_blocks;
}
