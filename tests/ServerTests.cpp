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

TEST(ServerTests, Status404ProducesNotFound) {
    Server::HttpResponse res;
    res.status = 404;
    res.body = "gone";
    EXPECT_NE(res.toString().find("HTTP/1.1 404 Not Found"), std::string::npos);
    std::cout << "Result determined!" << std::endl;
}

TEST(ServerTest, ContentLengthMatchesBodySize){
    Server::HttpResponse res;
    res.body = "Hello";
    EXPECT_NE(res.toString().find("Content-Length: 5"), std::string::npos);
}
