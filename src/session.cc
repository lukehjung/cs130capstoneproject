#include "session.h"
#include "echo_handler.h"
#include "static_file_handler.h"
#include "utils.h"

Utils utility;

session::session(boost::asio::io_service &io_service) : socket_(io_service)
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
    if (!request_start && !utility.complete(request, bytes_transferred) && utility.check_request(request))
    {
        http_body += request;
        good_request(request, getConfigLocation());
        return true;
    }
    else
        return false;
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
                good_request(request, getConfigLocation());
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

                    if (!utility.check_method(method_name))
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

// request handler
std::string session::good_request(std::string request, std::vector<std::string> ConfigLocation)
{
    INFO << "GOOD REQUEST:\n"
         << "\"" << request << "\"";

    std::string http_response = "";
    // static file request, handle by static file handler
    if (request.find("static") != std::string::npos)
    {
        // serve static file
        StaticFileHandler file_handler;
        file_handler.handler(this, request);
    }

    else // echo request
    {
        EchoHandler echo_handler;
        echo_handler.handler(this, request, true);
    }
    // reset http_body
    http_body = "\r\n\r\n";

    return http_response;
}

// Reformat the invalid request into the body of the reponse with status code 400
std::string session::bad_request(std::string &request)
{
    WARN << "BAD REQUEST:\n"
         << "\"" << request << "\"";
    std::string http_response = "";
    EchoHandler echo_handler;
    echo_handler.handler(this, request, false);
    // Reset the body
    http_body = "\r\n\r\n";
    // send_response(http_response);
    return http_response; // for testing
}

bool session::setConfigLocation(std::vector<std::string> configs)
{
    configLocation = configs;
}

std::vector<std::string> session::getConfigLocation()
{
    return configLocation;
}
