#include "gtest/gtest.h"
#include "request.h"
#include "response.h"
#include "proxy_handler.h"
#include <string>

class ProxyHandlerTest : public ::testing::Test
{
public:
    Response handle_request(Request::Method method, std::string path, std::string config_root, std::string uri) {
        p_handler.setLocation(path, config_root);
        req.method_ = method;
        req.uri_ = uri;
        req.version_ = "HTTP/1.1";
        rep = p_handler.handleRequest(req);
        return rep;
    }
private:
    Request req;
    Response rep;
    ProxyHandler p_handler;
};

TEST_F(ProxyHandlerTest, test_get_ucla_proxy)
{
    Response reply = handle_request(Request::GET, "/ucla", "www.ucla.edu", "/ucla");
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_get_ucla_proxy_favicon)
{
    Response reply = handle_request(Request::GET, "/ucla", "www.ucla.edu", "/ucla/favicon.ico");
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_get_ucla_proxy_about_page)
{
    Response reply = handle_request(Request::GET, "/ucla", "www.ucla.edu", "/ucla/about/");
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_get_google_proxy)
{
    Response reply = handle_request(Request::GET, "/google", "www.google.com", "/google");
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_get_unspecified_serve_addr)
{
    Response reply = handle_request(Request::GET, "", "www.google.com/", "/google");
    EXPECT_NE(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_get_google_proxy_search)
{
    Response reply = handle_request(Request::GET, "/google", "www.google.com", "/google/search");
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_get_nonexisting_proxy_path) 
{
    Response reply = handle_request(Request::GET, "/google", "www.google.com", "/google/hello");
    EXPECT_EQ(reply.code_, Response::not_found);
}

TEST_F(ProxyHandlerTest, test_post_ucla_proxy)
{
    Response reply = handle_request(Request::POST, "/ucla", "www.ucla.edu", "/ucla");
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, test_post_google_proxy)
{
    Response reply = handle_request(Request::POST, "/google", "www.google.com", "/google");
    EXPECT_EQ(reply.code_, Response::ok);
}
