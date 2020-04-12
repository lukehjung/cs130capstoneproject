//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.h"
#include "config_parser.h"

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

int getPortNum(const char* file_name)
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
    std::cerr << "ERROR: No port number found\n";
    exit(EXIT_FAILURE);
  }

  // if there are multiple port numbers, we pick the first one for now
  std::string first_port = port_queue.front();
  //port_queue.pop();

  // Check non-numeric port number 
  if (!std::all_of(first_port.begin(), first_port.end(), ::isdigit)) 
  {
    std::cerr << "ERROR: Non-numeric port number\n";
    exit(EXIT_FAILURE);
  } 
  return stoi(first_port);
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    using namespace std; // For stoi.
    int portNum = getPortNum(argv[1]);
    if (portNum < 1 || portNum > 65535)
    {
      std::cerr << "Invalid Port Number.\n";
      return 1;
    }

    server s(io_service, portNum);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
