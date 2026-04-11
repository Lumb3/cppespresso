// Executable main file
// Author: Erkhembileg Ariunbold
// Date: 2026.04.11

#include <iostream>
#include "Server.h"

constexpr int Default_port = 8080;
constexpr int Buffer_size = 4096; // amount of memory allocated in the server

int main (int argc, char* argv[]) {
    // Use default port if none provided
    int port = (argc >= 2) ? std::stoi(argv[1]) : Default_port;

    std::cout << "Listening on port: " << port << "..." << std::endl;
    try {
        Server::Connect(port);
    } catch (const std::exception& e) {
        std::cout << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}