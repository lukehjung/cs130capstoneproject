#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/xpressive/xpressive.hpp>  // for regex

using boost::asio::ip::tcp;
using boost::xpressive::sregex;

class session
{
public:
  // Constructor
  session(boost::asio::io_service& io_service);
  tcp::socket& socket() { return socket_; }
  void start();

private:
  void handle_read(const boost::system::error_code& error,
    size_t bytes_transferred);

  void handle_write(const boost::system::error_code& error);

  // Helper functions for request parser
  void send_response(std::string response);
  void good_request(std::string& body);
  void bad_request(std::string& body);
  bool check_method(std::string method);
  bool check_header(std::string header);
  bool complete(std::string request, size_t bytes_transferred);

  bool request_start;     // mark the start of the request
  std::string http_body;  // store one http request

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};
