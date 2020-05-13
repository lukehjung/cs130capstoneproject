#include "static_file_handler.h"

extern Utils utility;

std::map<std::string, std::string> StaticFileHandler::locationToRoot;

RequestHandler* StaticFileHandler::Init(const std::string& location_path, const NginxConfig& config)
{
  std::string path = config.ToString();
  // get the root path out of the enclosing quotation mark
  int pos = path.find("/");
  // get root directory; -3 because we need to ignore null terimnator, last quotation mark, and '/'
  std::string root = path.substr(pos, path.length() - pos - 3);

  // remove quotation marks from prefix
  std::string temp = location_path.substr(1, location_path.length() - 2);

  locationToRoot[temp] = root;
  RequestHandler* req_handler = new StaticFileHandler();
  return req_handler;
}

Response StaticFileHandler::handleRequest(const Request& request)
{
  std::string req = getRequestLine(request);

  // forming the request string
  for(auto const& header : request.headers_)
  {
    req += utility.format_header(header.first, header.second);
  }

  req += utility.format_end();
  req += request.body_;

  int src_type = configParser(req);
  std::string filename = utility.getContent(req);

  /*
  2: image/png
  3: image/jpeg
  4: application/octet-stream
  */
  if (src_type == 2 || src_type == 3 || src_type == 4)
  {
      // try other approach to send file
      return getBinaryContent(filename, src_type);
  }
  else // send plain text
  {
      // e.g. /static/subdir/hello.txt
      int pos = request.uri_.find("/") + 1;
      std::string location_path = request.uri_.substr(pos);
      std::string file_path = replace_path(location_path);

      if(!file_path.length())
      {
        Response res;
        INFO << "FILE NOT FOUND" << location_path;
        res.code_ = res.not_found;
        res.body_ = "Error: File Not Found";
        return res;
      }

      //std::string file_path = locationToRoot[location_prefix] + subpath + "/" + file;
      return formResponse(src_type, file_path);
  }
}

///////////////////////////////// New Code Added Above //////////////////////////////////////////////////////

void StaticFileHandler::handler(session *Session, std::string request)
{
  int src_type = configParser(request);
  std::string filename = utility.getContent(request);

  // send binary if the request mime is image or file
  if (src_type == 2 || src_type == 3 || src_type == 4)
  {
      // try other approach to send file
      send_binary(Session, filename, src_type);
  }
  else // send plain text
  {
      Session->send_response(getResponse(filename));
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

std::string StaticFileHandler::getResponse(std::string http_request)
{
    // HTTP Headers used for each type of file
    std::string text_header = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n";

    std::string not_found = "HTTP/1.0 404 Not Found\r\n"
                            "Connection: close\r\n\r\n"
                            "File Not Found\r\n";

    std::string http_response = "";

    // src_type is the int for which file is being requested
    // return_str is the name of the file
    int src_type = configParser(http_request);
    std::string return_str = replace_path(http_request);
    bool found = return_str.length();
    std::string body = "";

    std::string current_path = boost::filesystem::current_path().string();
    return_str = current_path + return_str;

    while (return_str[0] == '/' && return_str[1] == '/')
    {
        return_str = return_str.substr(1);
    }

    INFO << "CALLING FILE: " << return_str;

    if (!found)
    {
        INFO << "FILE NOT FOUND: " << return_str;
        return "Error: not found";
    }

    // if config is a GET request but no file
    else if (src_type == 0)
    {
        http_response = text_header;
        http_response += "Content-Length: 37\r\n\r\n";
        http_response += "Hello World! This is the index page.";
        return http_response;
    }

    // if config is html or txt
    else if (src_type == 1)
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

void StaticFileHandler::send_binary(session *Session, std::string filename, int src_type)
{
    INFO << "INSIDE GET_IMAGE";
    std::string http_response = "";

    std::string current_path = boost::filesystem::current_path().string();
    std::string return_str = replace_path(filename);
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

        if (src_type == 2)
        {
            http_response += utility.format_header("Content-type", "image/png");
        }
        else if (src_type == 3)
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

///////////////////////////////// New Code Added Below //////////////////////////////////////////////////////

Response StaticFileHandler::getBinaryContent(std::string filename, int src_type)
{
  INFO << "INSIDE GET_IMAGE";
  Response res;

  std::string current_path = boost::filesystem::current_path().string();
  std::string return_str = filename;
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
      fl.seekg(0, std::ios::beg);
      if (size) {
          fl.read(&image[0], size);
      }
      fl.close();

      res.code_ = res.ok;

      if (src_type == 2)
      {
          res.headers_["Content-type"] = "image/png";
      }
      else if (src_type == 3)
      {
          res.headers_["Content-type"] = "image/jpeg";
      }
      else
      {
          res.headers_["Content-type"] = "application/octet-stream";
      }

      res.headers_["Content-Length"] = std::to_string(size);
      res.headers_["Connection"] = "close";
      res.body_ = std::string(image.begin(), image.end());
  }

  else
  {
      res.code_ = res.bad_request;
      res.headers_["Connection"] = "close";
      //http_response += utility.format_end();
      INFO << "ERROR: " << return_str << " not found.";
  }

  return res;
}

Response StaticFileHandler::formResponse(int src_type, std::string file_path)
{
  Response res;

  // local_path: path of the file
  std::string current_path = boost::filesystem::current_path().string();
  std::string local_path = current_path + file_path;

  while (local_path[0] == '/' && local_path[1] == '/')
  {
    local_path = local_path.substr(1);
  }

  INFO << "CALLING FILE:" << local_path;

  // if config is a GET request but no file
  if (src_type == 0)
  {
      res.code_ = res.ok;
      res.headers_["Content-Type"] = "text/html; charset=UTF-8";
      res.headers_["Content-Length"] = "37";
      res.body_ = "Hello World! This is the index page.";
      return res;
  }

  // if config is html or txt file
  else if (src_type == 1)
  {
      res.code_ = res.ok;
      res.headers_["Content-Type"] = "text/html; charset=UTF-8";
      boost::filesystem::path my_path{ local_path };

      if (boost::filesystem::exists(my_path)) // only run if file is opened correctly
      {
          boost::filesystem::fstream fin(my_path, std::ios::in | std::ios::binary);

          std::string body = "";
          std::string line;
          while (std::getline(fin, line))
          {
              body += line;
          }

         res.headers_["Content-Length"] = std::to_string(body.size());
         res.body_ = body;
         fin.close();
      }

      else
      {
          INFO << "ERROR: " << local_path << " not found.";
          res.code_ = res.not_found;
          res.headers_["Connection"] = "close";
          res.body_ = "File Not FOUND";
      }

      return res;
  }
}

// this function will be moved to the newly created request_parser.cc later
std::string StaticFileHandler::getRequestLine(const Request& request) {
  int pos = request.uri_.find("/") + 1;
  std::string uri = request.uri_.substr(pos);
  return request.method_ + " " + uri + " " + request.version_ + "\r\n";
}

std::string StaticFileHandler::replace_path(const std::string& location_prefix)
{
  // e.g. hello.txt
  int pos = location_prefix.find_last_of("/") + 1;
  std::string file = location_prefix.substr(pos);

  // e.g. /static/subdir
  std::string prefix = location_prefix.substr(0, pos - 1);
  //longest prefix matching
  std::string subpath = "";
  while(!locationToRoot[prefix].length())
  {
    pos = prefix.find_last_of("/");

    if(pos == std::string::npos)
    {
      return "";
    }

    subpath = prefix.substr(pos) + subpath;
    prefix = prefix.substr(0, pos);
  }

  return locationToRoot[prefix] + subpath + "/" + file;
}
