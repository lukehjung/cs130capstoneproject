#include "session.h"  // this header already has logging.h and static_file_handler.h
#include "utils.h"

extern Utils utility;

void StaticFileHandler::handler(session *Session, std::string request)
{
  int config_type = configParser(request);
  std::string fileName = utility.getContent(request);

  // send binary if the request mime is image or file
  if (config_type == 2 || config_type == 3 || config_type == 4)
  {
      // try other approach to send file
      send_binary(Session, fileName, config_type);
  }
  else // send plain text
  {
      Session->send_response(getResponse(fileName, Session->configLocation));
  }

  Session->http_body = "\r\n\r\n";
}

/*
@ Session: current session
@ header: response header
@ content: image/file content
*/
void StaticFileHandler::dispatch(session *Session, std::string header, std::vector<char> content)
{
  Session->send_response(header);

  std::size_t size = content.size();
  std::size_t total_write {0};  // bytes successfully witten

  while (total_write != size ) {
      total_write += Session->socket_.write_some(boost::asio::buffer(&content[0] + total_write, size - total_write));
  }
}

int StaticFileHandler::configParser(std::string http_body)
{
    // regex for whole line, should include a GET request, then optional file, then http version
    // std::regex r("([A-Z]+ )\\/([a-zA-Z]+(\\.(txt|png|html|jpg|ico))*)*( HTTP\\/[1-2]\\.[0-2])(\\r\\n)");
    // if (std::regex_match(http_body.begin(), http_body.end(), r))
    // {
    // regex for each type of file that can be found
    // if not one of these, return 0
    std::regex txt_html("\\/([a-zA-Z]+\\.(txt|html))");
    std::regex jpg("\\/[a-zA-Z]+\\.jpg");
    std::regex png("\\/[a-zA-Z]+\\.png");
    std::regex favicon("\\/[a-zA-Z]+\\.ico");
    std::regex file("\\/[a-zA-Z]+");
    std::smatch m;
    if (std::regex_search(http_body, m, favicon))
    {
        return 0;
    }
    else if (std::regex_search(http_body, m, txt_html))
    {
        return 1;
    }
    else if (std::regex_search(http_body, m, png))
    {
        return 2;
    }
    else if (std::regex_search(http_body, m, jpg))
    {
        return 3;
    }
    else if (std::regex_search(http_body, m, file))
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

bool StaticFileHandler::parseAbsoluteRoot(std::string &location, std::vector<std::string> configLocation)
{
    std::string first_part = "/";
    std::string second_part = "";
    bool flag = false;
    for (int i = 1; i < location.length(); i++)
    {
        char c = location[i];
        if (c == '/')
        {
            first_part += c;
            second_part = location.substr(i);
            flag = true;
            break;
        }

        first_part += c;
    }
    if (!flag)
    {
        first_part = "/";
        second_part = location;
    }

    // iterate by 2 each time so add one after increasing it once

    for (int i = 0; i < configLocation.size(); i += 2)
    {
        // if found in configLocation, return true and set location variable to new prefix and append path
        if (first_part == configLocation[i])
        {
            location = configLocation[i + 1] + second_part;
            return true;
        }
    }

    // if couldn't find value in configLocation, return false
    return false;
}

std::string StaticFileHandler::getResponse(std::string http_request, std::vector<std::string> configLocation)
{
    // HTTP Headers used for each type of file
    std::string text_header = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n";

    std::string not_found = "HTTP/1.0 404 Not Found\r\n"
                            "Connection: close\r\n\r\n"
                            "File Not Found\r\n";

    std::string return_str = http_request,
                http_response = "";

    // config_type is the int for which file is being requested
    // return_str is the name of the file
    char ch;
    int config_type = configParser(http_request);
    bool found = parseAbsoluteRoot(return_str, configLocation);
    std::string body = "";

    std::string current_path = boost::filesystem::current_path().string();
    return_str = current_path + return_str;

    while (return_str[0] == '/' && return_str[1] == '/')
    {
        return_str = return_str.substr(1);
    }

    INFO << "CALLING FILE:" << return_str;

    if (!found)
    {
        INFO << "FILE NOT FOUND" << return_str;
        return "Error: not found";
    }

    // if config is a GET request but no file
    else if (config_type == 0)
    {
        http_response = text_header;
        http_response += "Content-Length: 37\r\n\r\n";
        http_response += "Hello World! This is the index page.";
        return http_response;
    }

    // if config is html or txt
    else if (config_type == 1)
    {

        boost::filesystem::path my_path{return_str};

        if (boost::filesystem::exists(my_path)) // only run if file is opened correctly
        {
            boost::filesystem::fstream fin(my_path, std::ios::in | std::ios::binary);
            http_response = text_header;

            body = "";
            std::string line;
            while (std::getline(fin, line))
            {
                body += line;
            }
           //http_response += text_header;
           http_response += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
           http_response += body;

           fin.close();
        }
        else
        {
            INFO << "ERROR: " << return_str << " not found.";
            return not_found;
        }
        return http_response;
    }
}

void StaticFileHandler::send_binary(session *Session, std::string fileName, int config_type)
{
    INFO << "INSIDE GET_IMAGE";
    std::string http_response = "";

    std::string current_path = boost::filesystem::current_path().string();
    std::string return_str = fileName;
    bool found = parseAbsoluteRoot(return_str, Session->configLocation);
    return_str = current_path + return_str;

    while (return_str[0] == '/' && return_str[1] == '/')
    {
        return_str = return_str.substr(1);
    }
    boost::filesystem::path my_path{return_str};

    if (boost::filesystem::exists(my_path)) // only run if file is opened correctly
    {
        std::ifstream fl(my_path.c_str());
        fl.seekg(0, std::ios::end);
        size_t size = fl.tellg();
        std::vector<char> image(size);
        //char image[size];
        fl.seekg(0, std::ios::beg);
        if (size) {
            fl.read(&image[0], size);
        }
        fl.close();

        http_response += utility.format_status("200 OK");

        if (config_type == 2)
        {
            http_response += utility.format_header("Content-type", "image/png");
        }
        else if (config_type == 3)
        {
            http_response += utility.format_header("Content-type", "image/jpeg");
        }
        else
        {
            http_response += utility.format_header("Content-type", "application/octet-stream");
        }
        http_response += utility.format_header("Content-length", std::to_string(size));
        http_response += utility.format_header("Connection", "close");
        http_response += utility.format_end();

        dispatch(Session, http_response, image);
        INFO << "SEND DATA SUCCESSFULLY";
    }
    else
    {
        http_response += utility.format_status("404 Not Found");
        http_response += utility.format_header("Connection", "close");
        http_response += utility.format_end();

        INFO << "ERROR: " << return_str << " not found.";
        Session->send_response(http_response);
    }
}