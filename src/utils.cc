#include "utils.h"

/* Request Handling */
int Utils::check_method(std::string method)
{
    if(method == "GET")
      return Request::GET;

    else if (method == "POST")
      return Request::POST;

    else if (method == "PUT")
      return Request::PUT;

    else if (method == "DELETE")
      return Request::DELETE;

    else if (method == "HEAD")
      return Request::HEAD;

    else if (method == "CONNECT")
      return Request::CONNECT;

    else if (method == "OPTIONS")
      return Request::OPTIONS;

    else if (method == "TRACE")
      return Request::TRACE;

    else if (method == "PATCH")
      return Request::PATCH;

    else
      return -1;
}

bool Utils::check_header(std::string header)
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
bool Utils::complete(std::string request, size_t bytes_transferred)
{
    return (request == "\r\n" && bytes_transferred == 2);
}

// Filter all the CRLFs
int Utils::filter_CRLF(std::string request)
{
    boost::replace_all(request, "\r\n", "");
    return request.length();
}

bool Utils::check_requestLine(std::string request)
{
    // This regex pattern is used for detecting http request line
    sregex request_line = sregex::compile("[A-Z]+\\s(\\/.*)\\s+(HTTP\\/[1-2]\\.[0-2])(\\r\\n)");
    if (boost::xpressive::regex_match(request, request_line))
    {
        return true;
    }

    return false;
}

bool Utils::check_request(std::string request)
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
    if (check_method(method)  < 0)
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

/* ------- Response Handling -------- */

std::string Utils::format_status(std::string status)
{
    return "HTTP/1.1 " + status + "\r\n";
}

std::string Utils::format_header(std::string key, std::string value)
{
    return key + ": " + value + "\r\n";
}

std::string Utils::format_end()
{
    return "\r\n";
}

// extract the echo content/filename
std::string Utils::getContent(std::string request)
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

    std::string content = request.substr(pos + 1, line - pos);
    int file_end_pos = content.find(" ");
    content = content.substr(0, file_end_pos);

    return content;
}

/* Reference https://www.boost.org/doc/libs/1_46_0/doc/html/boost_asio/example/http/server3/request_handler.cpp */
// Decode the http requests that contain any special character into string
std::string Utils::url_decode(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%') {
			if (i + 3 <= in.size()) {
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value) {
					out += static_cast<char>(value);
					i += 2;
				}
			}
		} else if (in[i] == '+') {
			out += ' ';
		} else {
			out += in[i];
		}
	}
	return out;
}

Response Utils::plain_text_response(const std::string& text, Response::StatusCode code) {
	Response res;
	res.body_ = text;
	res.code_ = code;
	res.headers_["Content-type"] = "text/plain";
	res.headers_["Content-length"] = std::to_string(text.length());
	res.headers_["Connection"] = "close";
  return res;
}
