cmake_minimum_required(VERSION 3.10.0)
project(low-key-web-server)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log regex)
find_package(Boost 1.50 REQUIRED COMPONENTS system)
find_package(Boost 1.50 REQUIRED COMPONENTS filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(port_lib src/port.cc)
add_library(static_file_lib src/static_file_handler.cc)
add_library(log_lib src/logging.cc)

# TODO(!): Update executable name, srcs, and deps
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver server_lib port_lib session_lib config_parser_lib log_lib static_file_lib Boost::system Boost::log_setup Boost::log Boost::filesystem)

# TODO(!): Update test executable name, srcs, and deps
add_executable(webserver_test tests/webserver_test.cc)
target_link_libraries(webserver_test server_lib port_lib session_lib config_parser_lib log_lib gtest_main static_file_lib Boost::system Boost::log_setup Boost::log Boost::filesystem)

add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test server_lib port_lib session_lib config_parser_lib log_lib gtest_main static_file_lib Boost::system Boost::log_setup Boost::log Boost::filesystem)

#add_executable(server_test tests/server_test.cc)
#target_link_libraries(server_test server_lib static_file_lib gtest_main Boost::log_setup Boost::log Boost::filesystem)

add_executable(port_test tests/port_test.cc)
target_link_libraries(port_test server_lib port_lib session_lib config_parser_lib log_lib static_file_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::filesystem)

add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test server_lib port_lib session_lib config_parser_lib log_lib static_file_lib gtest_main Boost::system Boost::log_setup Boost::log Boost::filesystem)

# TODO(!): Update with test binary
gtest_discover_tests(webserver_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)
#gtest_discover_tests(server_test WORKING DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(port_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# TODO(!): Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS webserver static_file_lib server_lib port_lib session_lib config_parser_lib TESTS webserver_test config_parser_test session_test port_test)

# non G-Test Test
add_test(NAME IntegrationTest COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh -s "$<TARGET_FILE:webserver>"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
