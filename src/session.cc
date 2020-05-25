#include "session.h"
#include "echo_handler.h"
#include "static_file_handler.h"
#include "error_handler.h"
#include "status_handler.h"
#include "utils.h"
#include "dispatcher.h"
#include "request_parser.h"

Utils utility;
RequestParser req_parser;

session::session(boost::asio::io_service &io_service, server *server) : socket_(io_service), server_(server)
{
    request_start = false;
    http_body = "";
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
    if (!request_start && !utility.complete(request, bytes_transferred) && utility.check_request(request))
    {
        http_body += request;
        good_request(http_body);
        return true;
    }
    else
    {
        return false;
    }
}

bool session::handle_read(const boost::system::error_code &error,
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

        else
        {
            // The request is complete
            if (request_start && utility.complete(request, bytes_transferred))
            {
                request_start = false;
                http_body += request;
                good_request(http_body);
                return true;
            }

            // Check if the http request is valid
            else if (request_start || utility.check_requestLine(request))
            {
                request_start = true;
                http_body += request;

                // This is mainly for performance consideration.
                // We only want to execute this block when we first encouter
                // the http request line and check its method type. Other than this,
                // we don't want to execute this block until the next http request.
                if (utility.check_requestLine(request))
                {
                    // get method name
                    int pos = request.find(" ");
                    std::string method_name = request.substr(0, pos);

                    if (utility.check_method(method_name) < 0)
                    {
                        request_start = false;
                        bad_request(http_body);
                    }
                    else
                    {
                        // Clear the buffer for next read
                        memset(data_, '\0', sizeof(char) * max_length);
                        handle_write(error);
                    }
                }
                // Check if header filed is valid
                else if (!utility.check_header(request))
                {
                    request_start = false;
                    bad_request(http_body);
                }

                // So far so good.
                else
                {
                    // Clear the buffer for next read
                    memset(data_, '\0', sizeof(char) * max_length);
                    handle_write(error);
                }
            }

            // Handle garbage input
            else
            {
                // Handle multiple consecutive CRLF input
                if (!request_start && utility.filter_CRLF(request) == 0)
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
        logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["clientIp"]).set("");
        INFO << "CLOSE CONNECTION";
        delete this;
        return false;
    }
}

bool session::handle_write(const boost::system::error_code &error)
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
        logging::attribute_cast<attrs::mutable_constant<std::string>>(logging::core::get()->get_global_attributes()["clientIp"]).set("");
        INFO << "CLOSE CONNECTION";
        delete this;
        return false;
    }
}


// Echo back the http request to the client
void session::send_response(std::string response)
{
    INFO << "Response to client:\n"
         << "\"" << response << "\"";
    // Clear the buffer for next http request
    memset(data_, '\0', sizeof(char) * max_length);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(response.c_str(), response.length()),
                             boost::bind(&session::handle_write, this,
                                         boost::asio::placeholders::error));
}


// Call request handlers here
std::string session::good_request(std::string request)
{
    INFO << "GOOD REQUEST:\n"
         << "\"" << request << "\"";

    /* Parse the request string into a request object here */
    std::tuple<RequestParser::Result, std::string::iterator> result = req_parser.Parse(request_, request.begin(), request.end());

    std::string prefix = request_.uri_;
    // add quotation marks to match config file format
    std::string temp = "\"" + prefix + "\"";
    RequestHandler *req_handler;
    bool found = true;
    int pos;

    while (server_->handlers_tackers.find(temp) == server_->handlers_tackers.end())
    {
        pos = prefix.find_last_of("/");

        if (pos == std::string::npos)
        {
            found = false;
            break;
        }

        prefix = prefix.substr(0, pos);
        // add double quotes becaues the key is enclosed with ""
        temp = "\"" + prefix + "\"";
    }

    /* Call corresponding handler */
    boost::promise<Response> p;
    boost::future<Response> f = p.get_future();
    boost::thread t1(boost::bind(&session::handler_task, this, found, temp, std::ref(p)));
    // detach the child thread here, main thread should not wait for it and go on
    t1.detach();
    Response response = f.get();
    dispatcher mailman(this);

    /* dispatch reposnse */
    mailman.dispatch(response);

    // reset http_body
    http_body = "";
    req_parser.reset(request_);
    return mailman.ToString(response.code_);
}

// Reformat the invalid request into the body of the reponse with status code 400
/* To do: need a handler to handle this bad request */
std::string session::bad_request(std::string &request)
{
    WARN << "BAD REQUEST:\n"
         << "\"" << request << "\"";

    EchoHandler echo_handler;
    //echo_handler.handler(this, request, false);

    boost::promise<Response> p;
    boost::future<Response> f = p.get_future();
    boost::thread t1(boost::bind(&EchoHandler::handler, echo_handler, this, request, false));
    INFO << "Child Thread: " << t1.get_id() << " handles bad request.";
    t1.detach();

    // Reset the body
    http_body = "";
    req_parser.reset(request_);
    return request;
}

void session::handler_task(bool found, std::string prefix, boost::promise<Response> &res)
{
  //boost::lock_guard<boost::mutex> lock{mutex_};
  INFO << "Child Thread: " << boost::this_thread::get_id() << " starts handling good request.";
  Response response;
  //std::string temp;
  if (!found)
  {
      INFO << "No Matching Handler Found.";
      prefix = "\"/\"";

      if(server_->handlers_tackers[prefix])
      {
        response = server_->handlers_tackers[prefix]->handleRequest(request_);
      }

      // not even error handler is registered
      else
      {
        ErrorHandler error_handler;
        response = error_handler.handleRequest(request_);
      }
  }

  else
  {
      response = server_->handlers_tackers[prefix]->handleRequest(request_);
  }

  INFO << "Child Thread: " << boost::this_thread::get_id() << " handles path " << prefix;
  res.set_value(response);
  INFO << "Child Thread: " << boost::this_thread::get_id() << " finishes handling good request.";
  //boost::this_thread::sleep_for(boost::chrono::milliseconds(10000));
}
