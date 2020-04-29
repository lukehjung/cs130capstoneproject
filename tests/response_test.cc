#include "gtest/gtest.h"
#include "response.h"


class ResponseDefaultResponseTest : public ::testing::Test {
  protected:
    void CheckContents(Response::ResponseCode status) {
      r = Response::DefaultResponse(status);

      ASSERT_EQ(status, r.GetStatus());
      ASSERT_EQ(DefaultResponse::ToHtml(r.GetStatus()), r.GetBody());
      ASSERT_EQ(2, r.GetHeaders().size());
      EXPECT_EQ("Content-Length", r.GetHeaders()[0].first);
      EXPECT_EQ(std::to_string(r.GetBody().size()), r.GetHeaders()[0].second);
      EXPECT_EQ("Content-Type", r.GetHeaders()[1].first);
      EXPECT_EQ("text/html", r.GetHeaders()[1].second);
    }
    Response r;
};

TEST_F(ResponseDefaultResponseTest, Ok) {
  CheckContents(Response::ResponseCode::ok);
}

TEST_F(ResponseDefaultResponseTest, Created) {
  CheckContents(Response::ResponseCode::created);
}

TEST_F(ResponseDefaultResponseTest, Accepted) {
  CheckContents(Response::ResponseCode::accepted);
}

TEST_F(ResponseDefaultResponseTest, NoContent) {
  CheckContents(Response::ResponseCode::no_content);
}

TEST_F(ResponseDefaultResponseTest, MovedPermanently) {
  CheckContents(Response::ResponseCode::moved_permanently);
}

TEST_F(ResponseDefaultResponseTest, MovedTemporarily) {
  CheckContents(Response::ResponseCode::moved_temporarily);
}

TEST_F(ResponseDefaultResponseTest, NotModified) {
  CheckContents(Response::ResponseCode::not_modified);
}

TEST_F(ResponseDefaultResponseTest, BadRequest) {
  CheckContents(Response::ResponseCode::bad_request);
}

TEST_F(ResponseDefaultResponseTest, Unauthorized) {
  CheckContents(Response::ResponseCode::unauthorized);
}

TEST_F(ResponseDefaultResponseTest, Forbidden) {
  CheckContents(Response::ResponseCode::forbidden);
}

TEST_F(ResponseDefaultResponseTest, NotFound) {
  CheckContents(Response::ResponseCode::not_found);
}

TEST_F(ResponseDefaultResponseTest, InternalServerError) {
  CheckContents(Response::ResponseCode::internal_server_error);
}

TEST_F(ResponseDefaultResponseTest, NotImplementederror) {
  CheckContents(Response::ResponseCode::not_implemented);
}

TEST_F(ResponseDefaultResponseTest, BadGateway) {
  CheckContents(Response::ResponseCode::bad_gateway);
}

TEST_F(ResponseDefaultResponseTest, ServiceUnavailable) {
  CheckContents(Response::ResponseCode::service_unavailable);
}

TEST(ResponseTest, HtmlResponse) {
  Response r = Response::HtmlResponse(
                         DefaultResponse::ToHtml(Response::ResponseCode::ok));

  ASSERT_EQ(Response::ResponseCode::ok, r.GetStatus());
  ASSERT_EQ(DefaultResponse::ToHtml(Response::ResponseCode::ok), r.GetBody());
  ASSERT_EQ(2, r.GetHeaders().size());
  EXPECT_EQ("Content-Length", r.GetHeaders()[0].first);
  EXPECT_EQ(std::to_string(r.GetBody().size()), r.GetHeaders()[0].second);
  EXPECT_EQ("Content-Type", r.GetHeaders()[1].first);
  EXPECT_EQ("text/html", r.GetHeaders()[1].second);
}

TEST(ResponseTest, PlainResponse) {
  Response r = Response::PlainTextResponse(
                         DefaultResponse::ToHtml(Response::ResponseCode::ok));

  ASSERT_EQ(Response::ResponseCode::ok, r.GetStatus());
  ASSERT_EQ(DefaultResponse::ToHtml(Response::ResponseCode::ok), r.GetBody());
  ASSERT_EQ(2, r.GetHeaders().size());
  EXPECT_EQ("Content-Length", r.GetHeaders()[0].first);
  EXPECT_EQ(std::to_string(r.GetBody().size()), r.GetHeaders()[0].second);
  EXPECT_EQ("Content-Type", r.GetHeaders()[1].first);
  EXPECT_EQ("text/plain", r.GetHeaders()[1].second);
}

TEST(ToStringTest, generaltest) {
  Response r = Response::PlainTextResponse(
                         DefaultResponse::ToHtml(Response::ResponseCode::ok));

  ASSERT_EQ(Response::ResponseCode::ok, r.GetStatus());
  ASSERT_EQ(DefaultResponse::ToHtml(Response::ResponseCode::ok), r.GetBody());
  ASSERT_EQ(2, r.GetHeaders().size());
  EXPECT_EQ("Content-Length", r.GetHeaders()[0].first);
  EXPECT_EQ(std::to_string(r.GetBody().size()), r.GetHeaders()[0].second);
  EXPECT_EQ("Content-Type", r.GetHeaders()[1].first);
  EXPECT_EQ("text/plain", r.GetHeaders()[1].second);
  EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-Length: 0\r\nContent-Type: text/plain\r\n\r\n", r.ToString());
}
