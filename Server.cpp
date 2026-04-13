/**
* @file Server.cpp
 * @brief  Server Class Implementation
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
#include <thread>
#include <mutex>


/**
 * Global mutex object to protect shared data
 */
std::mutex consoleMutex;
/**
 * A method for concurrently handling new client using threads.
 *
 * @param clientSocket
 */
void Server::HandleClient(int clientSocket) {
    // Read the HTTP request
    char buffer[Buffer_size] = {};
    read(clientSocket, buffer, sizeof(buffer) - 1);
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        clientCount++;
        std::cout << "Thread " << std::this_thread::get_id() << " handling request:\n"
        << buffer << std::endl;
    }

    // Send a basic response to each user
    std::string response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Connection: close\r\n"
      "\r\n"
      "Hello from my HTTP server!\n"
      "Active clients: " + std::to_string(clientCount);

    send(clientSocket, response.c_str(), response.size(), 0);
    ::close(clientSocket);
}

/**
 * @brief Starts the server by creating a socket and listening on the specified port.
 *
 * Initializes the server socket, binds it to the given port, and begins
 * listening for incoming client connections. The server runs continuously
 * until a termination signal (e.g., CTRL+C) triggers Disconnect().
 *
 * @param port The port number on which the server will listen for connections.
 */
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
    if (::bind(this->serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
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

        std::thread t(&Server::HandleClient, this, clientSocket);
        t.detach();
    }

    close(this->serverSocket);
    std::cout << "\nServer shut down" << std::endl;
}

/**
 * Disconnects the server by shutting down the current connection
 * and releasing associated resources. Marks the server as no longer
 * running and closes the server's active socket if it is open.
 */
void Server::Disconnect() {
    this->is_running = false;
    int fd = this->serverSocket.exchange(-1); // write serverSocket back to -1 after the condition
    if (fd != -1) { // Close the connection if server has socket
        close(fd);
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout<<"\n Total Client count: "  + std::to_string(clientCount) << std::endl;
        }
    }
}