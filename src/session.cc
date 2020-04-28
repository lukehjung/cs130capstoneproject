#include "session.h"
#include <string>

session::session(boost::asio::io_service& io_service) : socket_(io_service)
{
  request_start = false;
  http_body = "\r\n\r\n";
}

bool session::start()
{
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  return true;
}

bool session::long_string_handler(std::string request, size_t bytes_transferred)
{
  //If the request is passed in one long string, it should be handled here.
  if(!request_start && !complete(request, bytes_transferred) && check_request(request))
  {
    http_body += request;
    good_request(http_body);
    return true;
  }
  else return false;
}


bool session::handle_read(const boost::system::error_code& error,
  size_t bytes_transferred)
{
  if (!error)
  {
    std::string request(data_);

    //If the request is passed in one long string, it should be handled here.
    if (long_string_handler(request, bytes_transferred))
    {
      return true;
    }

    else {
      // The request is complete
      if(request_start && complete(request, bytes_transferred))
      {
        request_start = false;
        good_request(http_body);
        return true;
      }

      // Check if the http request is valid
      else if (request_start || check_requestLine(request))
      {
        request_start = true;
        http_body += request;

        // This is mainly for performance consideration.
        // We only want to execute this block when we first encouter
        // the http request line and check its method type. Other than this,
        // we don't want to execute this block until the next http request.
        if(check_requestLine(request))
        {
          // get method name
          int pos = request.find(" ");
          std::string method_name = request.substr(0, pos);

          if(!check_method(method_name))
          {
            request_start = false;
            bad_request(http_body);
          }
          else
          {
            // Clear the buffer for next read
            memset(data_, '\0', sizeof(char)*max_length);
            handle_write(error);
          }
        }
        // Check if header filed is valid
        else if(!check_header(request))
        {
          request_start = false;
          bad_request(http_body);
        }

        // So far so good.
        else
        {
          // Clear the buffer for next read
          memset(data_, '\0', sizeof(char)*max_length);
          handle_write(error);
        }
      }

      // Handle garbage input
      else
      {
        // Handle multiple consecutive CRLF input
        if(!request_start && filter_CRLF(request) == 0)
        {
          handle_write(error);
        }

        else
        {
          http_body += request;
          bad_request(http_body);
        }
      }
    }
  return true;
  }

  else
  {
    ERROR << error.message();
    delete this;
    return false;
  }
}

bool session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    return true;
  }
  else
  {
    ERROR << error.message();
    delete this;
    return false;
  }
}

// Echo back the http request to the client
void session::send_response(std::string response)
{
  INFO << "Response to client:\n" << "\"" << response << "\"";
  // Clear the buffer for next http request
  memset(data_, '\0', sizeof(char)*max_length);

  boost::asio::async_write(socket_,
      boost::asio::buffer(response.c_str(), response.length()),
      boost::bind(&session::handle_write, this,
        boost::asio::placeholders::error));

}

// Reformat the valid request into the body of the response with status code 200
std::string session::good_request(std::string& body)
{
  INFO << "GOOD REQUEST:\n" << "\"" << body << "\"";
  std::string status_line = "HTTP/1.1 200 OK\r\n";
  std::string header = "Content-Type: text/plain\r\n";
  std::string length = "Content-Length: " + std::to_string(filter_CRLF(body)) + "\r\n";
  std::string connection = "Connection: close";
  std::string response = status_line + header + length + connection + body;
  // Reset the body
  body = "\r\n\r\n";
  send_response(response);
  return response;  // for testing
}

// Reformat the invalid request into the body of the reponse with status code 400
std::string session::bad_request(std::string& body)
{
  WARN << "BAD REQUEST:\n" << "\"" << body << "\"";
  std::string status_line = "HTTP/1.1 400 Bad Request\r\n";
  std::string header = "Content-Type: text/plain\r\n";
  std::string length = "Content-Length: " + std::to_string(filter_CRLF(body)) + "\r\n";
  std::string connection = "Connection: close";
  std::string response = status_line + header + length + connection + body;

  // Reset the body
  body = "\r\n\r\n";
  send_response(response);
  return response;  // for testing
}

bool session::check_method(std::string method)
{
  if(method == "GET" || method == "PUT" || method == "POST" ||
     method == "HEAD" || method == "DELETE" || method == "OPTIONS" ||
     method == "TRACE" || method == "PATCH" || method == "CONNECT")
   {
     return true;
   }

   return false;
}

// Make sure the header field satisfies this pattern:
// message-header = field-name ":" [ field-value ] CRLF
bool session::check_header(std::string header)
{
  // Header field pattern
  sregex header_pattern = sregex::compile("([a-zA-Z]+(-([a-zA-z]+))*):\\s*(.*)");

  if(!boost::xpressive::regex_match(header, header_pattern))
  {
    return false;
  }

  return true;
}

// Whether the request is complete or not
bool session::complete(std::string request, size_t bytes_transferred)
{
  return (request == "\r\n" && bytes_transferred == 2);
}

// Filter all the CRLFs
int session::filter_CRLF(std::string request)
{
  boost::replace_all(request, "\r\n", "");
  return request.length();
}

bool session::check_requestLine(std::string request)
{
  // This regex pattern is used for detecting http request line
  sregex request_line = sregex::compile("[A-Z]+\\s(\\/.*)\\s+(HTTP\\/[1-2]\\.[0-2])(\\r\\n)");
  if(boost::xpressive::regex_match(request, request_line))
  {
    return true;
  }

  return false;
}

// This function is needed in cases the incoming request is passed
// in the form of long string. The request is a long string.
// We need to check each header field one by one and check if it completes.
bool session::check_request(std::string request)
{
  std::string delimiter = "\r\n";
  int line = 0;           //line pos
  int pos = 0;            //char pos
  bool isComplete = false;

  line = request.find(delimiter);
  std::string request_line = request.substr(0, line) + "\r\n";
  if(!check_requestLine(request_line))
  {
    return false;
  }

  // update the request
  request = request.substr(line + delimiter.length());

  // Check the method type in the request line
  pos = request_line.find(" ");
  std::string method = request_line.substr(0, pos);
  if(!check_method(method))
  {
    return false;
  }

  // Now run through the rest of the request
  // and check each header field
  line = request.find(delimiter);
  while(line != std::string::npos)
  {
    std::string header = request.substr(0, line);
    if(!check_header(header) && header != "")
    {
      return false;
    }

    //This if-statement will be executed when we ecnouter
    //two CRLFs in a row.
    if(header == "")
    {
      isComplete = !isComplete;
      break;
    }

    request = request.substr(line + delimiter.length());
    line = request.find(delimiter);
  }

  return isComplete;
}
