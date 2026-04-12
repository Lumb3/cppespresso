/** @brief  Server Class Implementation
* @author Erkhembileg Ariunbold
* @date: 2026.04.11
*/

#include "Server.h"
#include <sys/socket.h>
#include <stdexcept>
#include <string>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>

void Server::Connect(int port) {
    // 1. Create socket: IPv4 protocol (server address), TCP socket
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->serverSocket < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    // Allow port reuse to avoid "Address already in use" errors
    int opt = 1;
    setsockopt(this->serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Defining server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 3. Bind to port (create a unique listening port on a machine)
    if (bind(this->serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Failed to bind(listen) to port " + std::to_string(port));
    }

    // 4. Listen for connections
    if (listen(this->serverSocket, 10) < 0) { // maximums of 10 pending connections are allowed
        throw std::runtime_error("Failed to listen on socket");
    }

    // 5. Accept and handle connections in an infinite loop
    this->is_running = true;
    while(this->is_running) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLength = sizeof(clientAddr);

        int clientSocket = accept(this->serverSocket, (sockaddr*)& clientAddr, &clientAddrLength);
        if (clientSocket < 0) {
            if (!this->is_running) break; // Exit from the loop if server is not active anymore
            std::cout << "Failed to accept connection" << std::endl;
            continue;
        }

        // Read the HTTP request
        char buffer[Buffer_size] = {};
        read(clientSocket, buffer, sizeof(buffer) - 1);

        // Send a basic HTTP response
        std::string response = "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello from my HTTP server!";
        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
    }

    close(this->serverSocket);
    std::cout << "\nServer shut down" << std::endl;
}

void Server::Disconnect() {
    this->is_running = false;
    if (this->serverSocket != -1) { // Close the connection if server has socket
        close(this->serverSocket);
        this->serverSocket = -1;
    }
}