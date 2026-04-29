/**
* @file tests/ServerTests.cpp
 * @brief Unit tests for request parsing and routing.
 */

#include "gtest/gtest.h"
#include "../Server.h"


TEST(ServerTests, Status200ProducesOK) {
    Server::HttpResponse res;
    res.status = 200;
    res.body = "hello";
    EXPECT_NE(res.toString().find("HTTP/1.1 200 OK"), std::string::npos);
}
