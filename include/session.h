#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/xpressive/xpressive.hpp>       // for regex
#include <boost/algorithm/string/replace.hpp>  // for replace_all

#include "logging.h"

using boost::asio::ip::tcp;
using boost::xpressive::sregex;

class session
{
public:
  // Constructor
  session(boost::asio::io_service& io_service);
  tcp::socket& socket() { return socket_; }
  bool start();

  bool long_string_handler(std::string request, size_t bytes_transferred);
  bool handle_read(const boost::system::error_code& error,
    size_t bytes_transferred);

  bool handle_write(const boost::system::error_code& error);

  // Helper functions for request parser
  void send_response(std::string response);
  std::string good_request(std::string& body);
  std::string bad_request(std::string& body);
  bool check_requestLine(std::string request);
  bool check_method(std::string method);
  bool check_header(std::string header);
  bool complete(std::string request, size_t bytes_transferred);
  bool check_request(std::string request);
  int filter_CRLF(std::string request);

  bool request_start;     // mark the start of the request
  std::string http_body;  // store one http request

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};
