#include "session.h"

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
    if (!request_start && !complete(request, bytes_transferred) && check_request(request))
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
            if (request_start && complete(request, bytes_transferred))
            {
                request_start = false;
                good_request(request, getConfigLocation());
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
                if (check_requestLine(request))
                {
                    // get method name
                    int pos = request.find(" ");
                    std::string method_name = request.substr(0, pos);

                    if (!check_method(method_name))
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
                else if (!check_header(request))
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
                if (!request_start && filter_CRLF(request) == 0)
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
        ERROR << "CLOSE CONNECTION";
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
    // static file request
    if (request.find("static") != std::string::npos) 
    {
        // serve static file 
        StaticFileHandler file_handler;
        int config_type = file_handler.configParser(request);
        std::vector<std::string> fileMap = ConfigLocation;
        std::string fileName = getFileName(request);

        // send binary if the request mime is image or file
        if (config_type == 2 || config_type == 3 || config_type == 4) 
        {
            // try other approach to send file
            send_binary(fileName, config_type);
        } 
        else // send plain text
        {
            http_response = file_handler.getResponse(fileName, fileMap);
            send_response(http_response);
        }
    }
    else // echo request
    {
        http_response += format_status("200 OK");
        http_response += format_header("Content-length", std::to_string(request.length()));
        http_response += format_header("Connection", "close");
        http_response += format_end();
        http_response += request;

        send_response(http_response);
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
    http_response += format_status("400 Bad Request");
    http_response += format_header("Content-length", std::to_string(request.length()));
    http_response += format_header("Connection", "close");
    http_response += format_end();
    http_response += request;

    // Reset the body
    http_body = "\r\n\r\n";
    send_response(http_response);
    return http_response; // for testing
}

bool session::check_method(std::string method)
{
    if (method == "GET" || method == "PUT" || method == "POST" ||
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

    if (!boost::xpressive::regex_match(header, header_pattern))
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
    if (boost::xpressive::regex_match(request, request_line))
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
    int line = 0; //line pos
    int pos = 0;  //char pos
    bool isComplete = false;

    line = request.find(delimiter);
    std::string request_line = request.substr(0, line) + "\r\n";
    if (!check_requestLine(request_line))
    {
        return false;
    }

    // update the request
    request = request.substr(line + delimiter.length());

    // Check the method type in the request line
    pos = request_line.find(" ");
    std::string method = request_line.substr(0, pos);
    if (!check_method(method))
    {
        return false;
    }

    // Now run through the rest of the request
    // and check each header field
    line = request.find(delimiter);
    while (line != std::string::npos)
    {
        std::string header = request.substr(0, line);
        if (!check_header(header) && header != "")
        {
            return false;
        }

        //This if-statement will be executed when we encounter
        //two CRLFs in a row.
        if (header == "")
        {
            isComplete = !isComplete;
            break;
        }

        request = request.substr(line + delimiter.length());
        line = request.find(delimiter);
    }

    return isComplete;
}

bool session::setConfigLocation(std::vector<std::string> configs)
{
    configLocation = configs;
}

std::vector<std::string> session::getConfigLocation()
{
    return configLocation;
}

std::string session::getFileName(std::string request)
{
    std::string delimiter = "\r\n";
    int line = 0; //line pos
    int pos = 0;  //char pos

    line = request.find(delimiter);
    std::string request_line = request.substr(0, line) + "\r\n";

    // update the request
    request = request.substr(0, line);

    // Check the method type in the request line
    pos = request_line.find(" ");

    std::string file_name = request.substr(pos + 1, line - pos);
    int file_end_pos = file_name.find(" ");
    file_name = file_name.substr(0, file_end_pos);

    return file_name;
}

void session::send_binary(std::string fileName, int config_type)
{
    INFO << "INSIDE GET_IMAGE";
    StaticFileHandler file_handler;
    std::string http_response = "";

    std::string current_path = boost::filesystem::current_path().string();
    std::string return_str = fileName;
    bool found = file_handler.parseAbsoluteRoot(return_str, configLocation);
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

        http_response += format_status("200 OK");

        if (config_type == 2)
        {
            http_response += format_header("Content-type", "image/png");
        }
        else if (config_type == 3)
        {
            http_response += format_header("Content-type", "image/jpeg");
        }
        else
        {
            http_response += format_header("Content-type", "application/octet-stream");
        }
        http_response += format_header("Content-length", std::to_string(size));
        http_response += format_header("Connection", "close");
        http_response += format_end();

        // send response first
        send_response(http_response);

        // send data
        std::size_t total_write {0};  // bytes successfully witten

        while (total_write != size ) {
            total_write += socket_.write_some(boost::asio::buffer(&image[0]+total_write, size - total_write));
        }

        INFO << "SEND DATA SUCCESSFULLY";
    }
    else
    {
        http_response += format_status("404 Not Found");
        http_response += format_header("Connection", "close");
        http_response += format_end();

        INFO << "ERROR: " << return_str << " not found.";
        send_response(http_response);

    }

}

std::string session::format_status(std::string status) 
{
    return "HTTP/1.1 " + status + "\r\n";
}

std::string session::format_header(std::string key, std::string value) 
{
    return key + ": " + value + "\r\n";
}

std::string session::format_end()
{
    return "\r\n";
}