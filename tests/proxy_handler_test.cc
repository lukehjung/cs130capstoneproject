#include "gtest/gtest.h"
#include "request.h"
#include "response.h"
#include "proxy_handler.h"
#include <string>

class ProxyHandlerTest : public ::testing::Test
{
public:
    Response handle_request(Request::Method method, std::string path, std::string config_root, std::string uri)
    {
        p_handler.setLocation(path, config_root);
        req.method_ = method;
        req.uri_ = uri;
        req.version_ = "HTTP/1.1";
        rep = p_handler.handleRequest(req);
        return rep;
    }

    std::string parse_html_body(std::string &msg)
    {
        p_handler.setLocation("/ucla", "www.ucla.edu");
        return p_handler.parse_html_body(msg);
    }

    std::map<std::string, int> parse_cache_hdrs(std::map<std::string, std::string> req_hdrs)
    {
        return p_handler.parse_cache_hdrs(req_hdrs);
    }

    bool cache_control(std::map<std::string, int> cache_hdrs, const std::string req_uri,
                                     const bool must_validate, bool &can_use, bool &cache_only, bool &should_cache)
    {
        p_handler.store_cache("www.ucla.edu", rep);
        return p_handler.cache_control(cache_hdrs, req_uri, must_validate, can_use, cache_only, should_cache);
    }

    Response use_cache(std::string req_uri)
    {
        p_handler.setLocation("/ucla", "www.ucla.edu");
        req.method_ = Request::GET;
        req.uri_ = "/ucla";
        req.version_ = "HTTP/1.1";
        rep = p_handler.handleRequest(req);

        p_handler.store_cache("www.ucla.edu", rep);

        return p_handler.use_cache(req_uri);
    }

    Response no_cache()
    {
        return p_handler.no_cache();
    }

    int test_store_cache(std::string req_uri, Response res)
    {
        p_handler.store_cache(req_uri, res);
        return p_handler.get_cached_pages().count(req_uri);
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

TEST_F(ProxyHandlerTest, testParseHTMLBody)
{
    std::string html = "\"/about";
    std::string ret = parse_html_body(html);
    ret = ret.substr(1, ret.length()); //removes extra \"
    EXPECT_EQ("/ucla/about", ret);
}

TEST_F(ProxyHandlerTest, testParseCacheHdrs)
{
    std::map<std::string, std::string> input;
    std::map<std::string, int> real_output;
    std::map<std::string, int> expected_output;
    input = {{"Cache-Control", "max-age=100, min-fresh=10, only-if-cached"}};
    expected_output = {{"max-age", 100}, {"min-fresh", 10}, {"only-if-cached", 1}};

    real_output = parse_cache_hdrs(input);

    EXPECT_EQ(real_output, expected_output);
}

TEST_F(ProxyHandlerTest, TestCacheControl_1)
{
    std::map<std::string, int> cache_hdrs = {{"max-age", 100}, {"min-fresh", 10}, {"only-if-cached", 1}};
    const std::string req_uri = "www.ucla.edu";
    const bool must_validate = false;
    bool can_use = true;
    bool cache_only = false;
    bool should_cache = true;

    EXPECT_FALSE(cache_control(cache_hdrs, req_uri, must_validate, can_use, cache_only, should_cache)); 
}

TEST_F(ProxyHandlerTest, TestCacheControl_2)
{
    std::map<std::string, int> cache_hdrs = {{"max-age", 1}, {"min-fresh", 10}, {"only-if-cached", 1}};
    const std::string req_uri = "www.ucla.edu";
    const bool must_validate = false;
    bool can_use = false;
    bool cache_only = false;
    bool should_cache = true;

    EXPECT_FALSE(cache_control(cache_hdrs, req_uri, must_validate, can_use, cache_only, should_cache));
}

TEST_F(ProxyHandlerTest, TestCacheControl_3)
{
    std::map<std::string, int> cache_hdrs = {{"max-age", 1}, {"min-fresh", 10}, {"only-if-cached", 1}};
    const std::string req_uri = "www.ucla.edu";
    const bool must_validate = true;
    bool can_use = false;
    bool cache_only = false;
    bool should_cache = true;

    EXPECT_TRUE(cache_control(cache_hdrs, req_uri, must_validate, can_use, cache_only, should_cache));
}

TEST_F(ProxyHandlerTest, testUseCache)
{
    std::string url = "www.ucla.edu";
    Response reply = use_cache(url);
    EXPECT_EQ(reply.code_, Response::ok);
}

TEST_F(ProxyHandlerTest, testNoCache)
{
    Response reply = no_cache();
    EXPECT_EQ(reply.code_, Response::gateway_timeout);
}

TEST_F(ProxyHandlerTest, testStoreCache)
{
    Response rep;
    EXPECT_TRUE(test_store_cache("www.ucla.edu", rep) > 0);
}

TEST_F(ProxyHandlerTest, testSetLocation)
{
    Response rep;
    EXPECT_TRUE(test_store_cache("www.ucla.edu", rep) > 0);
}
