#include "gtest/gtest.h"
#include "request.h"

TEST(RequestParser, ValidRequest) {
  std::string test_string = "GET /echo%20hello HTTP/1.1\r\n"
                            "Host: localhost:8081\r\n"
                            "User-Agent: testing\r\n"
                            "Accept: -/-\r\n"
                            "\r\n";

  std::unique_ptr<Request> request = Request::Parse(test_string);
  ASSERT_TRUE(request != NULL);
  EXPECT_EQ(Request::GetParseResult(), Request::good);
  EXPECT_EQ(test_string, request->raw_request());
  EXPECT_EQ("GET", request->method());
  EXPECT_EQ("/echo hello", request->path());
  EXPECT_EQ("/echo%20hello", request->uri());
  EXPECT_EQ("HTTP/1.1", request->version());

  std::vector<std::pair<std::string, std::string> > headers;
  headers = request->headers();
  ASSERT_EQ(3, headers.size()) << "There should be 3 headers";

  EXPECT_EQ("Host", headers[0].first);
  EXPECT_EQ("localhost:8081", headers[0].second);

  EXPECT_EQ("User-Agent", headers[1].first);
  EXPECT_EQ("testing", headers[1].second);

  EXPECT_EQ("Accept", headers[2].first);
  EXPECT_EQ("-/-", headers[2].second);
}


TEST(RequestParser, ValidBodyRequest) {
  std::string test_string = "GET /echo%20hello HTTP/1.1\r\n"
                            "Host: localhost:8081\r\n"
                            "User-Agent: testing\r\n"
                            "Content-Length: 5\r\n"
                            "Accept: -/-\r\n"
                            "\r\nHELLO";
  std::unique_ptr<Request> request = Request::Parse(test_string);
  ASSERT_TRUE(request != NULL);

  EXPECT_EQ(Request::GetParseResult(), Request::good);
  EXPECT_EQ(test_string, request->raw_request());
  EXPECT_EQ("GET", request->method());
  EXPECT_EQ("/echo hello", request->path());
  EXPECT_EQ("/echo%20hello", request->uri());
  EXPECT_EQ("HTTP/1.1", request->version());

  std::vector<std::pair<std::string, std::string> > headers;
  headers = request->headers();
  ASSERT_EQ(4, headers.size()) << "There should be 4 headers";

  EXPECT_EQ("Host", headers[0].first);
  EXPECT_EQ("localhost:8081", headers[0].second);
  EXPECT_EQ("User-Agent", headers[1].first);
  EXPECT_EQ("testing", headers[1].second);
  ASSERT_EQ("Content-Length", headers[2].first);
  ASSERT_EQ("5", headers[2].second);
  EXPECT_EQ("Accept", headers[3].first);
  EXPECT_EQ("-/-", headers[3].second);
  EXPECT_EQ("HELLO", request->body());
}

TEST(RequestParser, UndefinedRequest) {
  std::string test_string = "GET /echo HT";
  std::unique_ptr<Request> request = Request::Parse(test_string);
  ASSERT_TRUE(request == NULL);
  EXPECT_EQ(Request::GetParseResult(), Request::undefined);
}

TEST(RequestParser, BadRequest) {
  std::string test_string = "This is\r\n"
                            "random text\r\n";
  std::unique_ptr<Request> request = Request::Parse(test_string);
  ASSERT_TRUE(request == NULL);
  EXPECT_EQ(Request::GetParseResult(), Request::bad);
}
