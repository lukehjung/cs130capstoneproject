#include "../server.h"
#include <gmock/gmock.h>
#include <signal.h>
#include <boost/bind.hpp>
using ::testing::AtLeast;

// https://stackoverflow.com/questions/32529009/google-mock-how-to-name-mock-functions
class MockServer :  public Server{
  public:
    explicit MockServer() : http::server::Server("35.230.115.120",
                                                 "8080",
                                                 "") {}
    MOCK_METHOD0(run, void());
    MOCK_METHOD0(start_accept, void());
    MOCK_METHOD0(handle_stop, void());
};

// https://github.com/google/googletest/blob/master/googlemock/docs/for_dummies.md
TEST(ServerTest, runTest){
  MockServer mockServer;
  EXPECT_CALL(mockServer, start_accept()).Times(AtLeast(0));
  EXPECT_CALL(mockServer, run()).Times(AtLeast(1));
  EXPECT_CALL(mockServer, handle_stop()).Times(AtLeast(0));
  mockServer.run();
}
