#include "static_file_handler.h"
#include "status_handler.h"

extern Utils utility;
StatusHandler status_handler;

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
  //std::string filename = utility.getContent(req);

  /*
  2: image/png
  3: image/jpeg
  4: application/octet-stream
  */
  if (src_type == 2 || src_type == 3 || src_type == 4)
  {
      // try other approach to send file
      std::string file_path = replace_path(request.uri_);
      return getBinaryContent(file_path, src_type);
  }
  else // send plain text
  {
      // e.g. /static/subdir/hello.txt
      std::string file_path = replace_path(request.uri_);

      if(!file_path.length())
      {
        Response res;
        INFO << "FILE NOT FOUND" << request.uri_;
        res.code_ = res.not_found;
        res.body_ = "Error: File Not Found";
        status_handler.addRecord(request.uri_, "StaticHandler", Response::not_found);
        return res;
      }
      //std::string file_path = locationToRoot[location_prefix] + subpath + "/" + file;
      return formResponse(src_type, file_path);
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

Response StaticFileHandler::getBinaryContent(std::string filename, int src_type)
{
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

      res.code_ = Response::ok;

      if (src_type == 2)
      {
          res.src_type = 2;
          res.headers_["Content-type"] = "image/png";
      }
      else if (src_type == 3)
      {
          res.src_type = 3;
          res.headers_["Content-type"] = "image/jpeg";
      }
      else
      {
          res.src_type = 4;
          res.headers_["Content-type"] = "application/octet-stream";
      }

      res.headers_["Content-Length"] = std::to_string(size);
      res.headers_["Connection"] = "close";
      res.body_ = std::string(image.begin(), image.end());
  }

  else
  {
      res.code_ = Response::not_found;
      res.headers_["Connection"] = "close";
      INFO << "ERROR: " << return_str << " not found.";
  }
  status_handler.addRecord(filename, "StaticHandler", res.code_);

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
      res.code_ = Response::ok;
      res.src_type = 0;
      res.headers_["Content-Type"] = "text/html; charset=UTF-8";
      res.headers_["Content-Length"] = "37";
      res.headers_["Connection"] = "close";
      res.body_ = "Hello World! This is the index page.";
      status_handler.addRecord(file_path, "StaticHandler", Response::ok);

      return res;
  }

  // if config is html or txt file
  else if (src_type == 1)
  {
      res.code_ = Response::ok;
      res.src_type = 1;
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
          res.code_ = Response::not_found;
          res.headers_["Connection"] = "close";
          res.body_ = "File Not FOUND";
      }
      status_handler.addRecord(file_path, "StaticHandler", res.code_);

      return res;
  }
}

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
