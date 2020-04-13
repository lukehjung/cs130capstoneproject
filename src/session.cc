#include "session.h"
#include <iostream>
#include <string>

session::session(boost::asio::io_service& io_service) : socket_(io_service)
{
  request_start = false;
  http_body = "";
}

void session::start()
{
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
  size_t bytes_transferred)
{
  if (!error)
  {
    std::string request(data_);
    // This regex pattern is used for detecting http request line
    sregex request_line = sregex::compile("[A-Z]+\\s(\\/.*)\\s+(HTTP\\/[1-2]\\.[0-2])(\\r\\n)");

    // The request is complete
    // Either newline or CRLF would tell the server that the request is complete
    if(request_start && complete(request, bytes_transferred))
    {
      request_start = false;
      good_request(http_body);
    }

    // Check if the http request is valid
    else if (request_start || boost::xpressive::regex_match(request, request_line))
    {
      request_start = true;
      http_body += request;

      // This is mainly for performance consideration.
      // We only want to execute this block when we first encouter
      // the http request line and check its method type. Other than this,
      // we don't want to execute this block until the next http request.
      if(boost::xpressive::regex_match(request, request_line))
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
      if(!request_start && complete(request, bytes_transferred))
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
  else
  {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    delete this;
  }
}

// Echo back the http request to the client
void session::send_response(std::string response)
{
  // Clear the buffer for next http request
  memset(data_, '\0', sizeof(char)*max_length);

  boost::asio::async_write(socket_,
      boost::asio::buffer(response.c_str(), response.length()),
      boost::bind(&session::handle_write, this,
        boost::asio::placeholders::error));
}

// Reformat the valid request into the body of the response with status code 200
void session::good_request(std::string& body)
{
  std::string status_line = "HTTP/1.1 200 OK";
  std::string header = "Content-Type: text/plain";
  std::string response = status_line + "\n" + header + body;
  // Reset the body
  body = "";
  send_response(response);
}

// Reformat the invalid request into the body of the reponse with status code 400
void session::bad_request(std::string& body)
{
  std::string status_line = "HTTP/1.1 400 Bad Request";
  std::string header = "Content-Type: text/plain";
  std::string response = status_line + "\n" + header + body;
  // Reset the body
  body = "";
  send_response(response);
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
  sregex header_pattern = sregex::compile("([a-zA-Z]+(-([a-zA-z]+))*):\\s*(.*)(\\r\\n)");

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
