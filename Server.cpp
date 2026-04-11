// Server class implementation
// Author: Erkhembileg Ariunbold
// Date: 2026.04.11

#include "Server.h"
#include <sys/socket.h>
#include <stdexcept>
#include <string>
#include <netinet/in.h>

void Server::Connect(int port) {
    // 1. Create socket: IPv4 protocol (server address), TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    // Allow port reuse to avoid "Address already in use" errors
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Defining server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 3. Bind to port (create a unique listening port on a machine)
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Failed to bind(listen) to port " + std::to_string(port));
    }

    // 4. Listen for connections
    if (listen(serverSocket, 10) < 0) { // maximums of 10 pending connections are allowed
        throw std::runtime_error("Failed to listen on socket");
    }

    // 5. Accept and handle connections in an infinite loop
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLength = sizeof(clientAddr);
    }
}
