/**
 * @file Server.cpp
 * @brief Server class implementation.
 * @author Erkhembileg Ariunbold
 * @date 2026-04-11
 */

#include "Server.h"

#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <fstream>


// Client handling
void Server::HandleClient(int clientSocket) {
    // Prevent a slow or stalled client from holding a worker thread forever.
    struct timeval timeout{5, 0};
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    std::string raw;
    char buffer[Buffer_size];

    // Read in a loop: a single read() call may not deliver the full request.
    while (true) {
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            // Connection closed early or timeout — nothing to respond to.
            close(clientSocket);
            return;
        }
        buffer[bytesRead] = '\0'; // null at the end of the buffer
        raw += buffer;

        // Wait until the blank line separating headers from body has arrived.
        auto headerEnd = raw.find("\r\n\r\n");
        if (headerEnd == std::string::npos) continue; // If there seperator is not found, skip the iteration

        // Once headers are complete, determine how many body bytes are expected.
        size_t contentLength = 0;
        auto clPos= raw.find("Content-Length: ");
        if (clPos != std::string::npos) {
            contentLength = std::stoul(raw.substr(clPos + 16)); // Get the length of the content
        }

        // Stop reading once the declared body length has been received.
        size_t bodyReceived = raw.size() - (headerEnd + 4);
        if (bodyReceived >= contentLength) break;
    }

    auto request  = parseRequest(raw);
    auto response = handleRoute(request);

    std::string responseStr = response.toString();
    send(clientSocket, responseStr.c_str(), responseStr.size(), 0);
    close(clientSocket);
}


// Request parsing
Server::HttpRequest Server::parseRequest(const std::string& raw) {
    HttpRequest req;

    // First line: "METHOD path HTTP/1.1"
    auto firstLineEnd = raw.find("\r\n");
    std::string firstLine = raw.substr(0, firstLineEnd);

    size_t methodEnd = firstLine.find(' ');
    size_t pathEnd = firstLine.find(' ', methodEnd + 1);

    req.method = firstLine.substr(0, methodEnd);
    req.path = firstLine.substr(methodEnd + 1, pathEnd - methodEnd - 1);

    // Everything before the blank line is the header block; everything after is the body.
    auto headerEnd = raw.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        req.headers = raw.substr(0, headerEnd);
        req.body = raw.substr(headerEnd + 4);
    }

    return req;
}

// Resolve the requested path
std::string Server::ResolvePath(const std::string &requestedPath) {
    if (requestedPath == "/index") {
        return "./public/index.html";
    }
    return "/"; // Return the default path if not found
}

Server::HttpResponse Server::serveFile(const std::string &path) {
    HttpResponse res;
    if (path == "/") {
        return res;
    }
    std::ifstream file(path, std::ios::binary);
    if (file) {
        res.body = std::string(std::istreambuf_iterator<char>(file), {});
        res.contentType = getMimeType(path);
    } else {
        res.status = 404;
        res.body = "Not Found";
    }
    return res;
}

// Routing
Server::HttpResponse Server::handleRoute(const HttpRequest& req) {
    HttpResponse res;

    if (req.method == "POST" && req.path == "/about") {
        res.body = "Received POST from about page:\n" + req.body;
    } else if (req.path == "/health") {
        res.body = "OK";
    } else if (req.method == "GET") {
        std::string filePath = this->ResolvePath(req.path);

        if (filePath.find("..") != std::string::npos) {
            res.status = 403;
            res.body = "Forbidden";
            return res;
        }
        return this->serveFile(filePath);
    }
    else {
        res.status = 404;
        res.body   = "Not Found";
    }

    return res;
}

// Maps a file extension to its MIME type for the Content-Type header.
std::string Server::getMimeType(const std::string& path) {
    auto dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";

    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css")                   return "text/css";
    if (ext == ".js")                    return "application/javascript";
    if (ext == ".json")                  return "application/json";
    if (ext == ".png")                   return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".svg")                   return "image/svg+xml";
    if (ext == ".ico")                   return "image/x-icon";
    if (ext == ".txt")                   return "text/plain";
    return "application/octet-stream";  // safe default for unknown types
}

// Thread pool
void Server::WorkerLoop() {
    while (poolRunning) {
        int clientSocket = -1;
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            // Sleep until there is work to do or the pool is shutting down.
            queueCV.wait(lock, [this] {
                return !taskQueue.empty() || !poolRunning;
            });

            // Exit if the pool is stopping and there is nothing left to handle.
            if (!poolRunning && taskQueue.empty()) return;

            clientSocket = taskQueue.front(); // initialize the most recent work's ClientSocket
            taskQueue.pop();
        }

        HandleClient(clientSocket);
    }
}

// Lifecycle
void Server::Connect(int port) {
    // Create a TCP/IPv4 socket.
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->serverSocket < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    // Allow immediate reuse of the port after the server restarts.
    int opt = 1;
    setsockopt(this->serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to all interfaces on the requested port.
    sockaddr_in serverAddress{};
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_port        = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (::bind(this->serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Failed to bind to port " + std::to_string(port));
    }

    // Allow up to 10 connections to queue while a worker is busy.
    if (::listen(this->serverSocket, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket.");
    }

    // Start the worker pool before entering the accept loop so no connection
    // is queued without a thread ready to pick it up.
    poolRunning = true;
    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        workerThreads.emplace_back(&Server::WorkerLoop, this);
    }

    std::clog << "Server listening on port " << port << std::endl;

    // Accept loop — blocks until Disconnect() closes the socket.
    this->is_running = true;
    while (this->is_running) {
        sockaddr_in clientAddr{};
        socklen_t   clientAddrLen = sizeof(clientAddr);

        int clientSocket = ::accept(
            this->serverSocket,
            (sockaddr*)&clientAddr,
            &clientAddrLen
        );

        if (clientSocket < 0) {
            if (!this->is_running) break; // Normal shutdown path.
            std::clog << "Failed to accept connection \n";
            continue;
        }

        // Hand the socket off to the pool without blocking the accept loop.
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push(clientSocket); // put the socket in the queue
        }
        queueCV.notify_one(); // wake exactly one sleeping worker
    }

    ::close(this->serverSocket);
    std::clog << "\nServer shut down." << std::endl;
}

void Server::Disconnect() {
    this->is_running = false;

    // Atomically swap the fd to -1 so no other thread closes it a second time.
    int fd = this->serverSocket.exchange(-1);
    if (fd != -1) {
        close(fd);
    }

    // Signal all workers to wake up and check poolRunning, then join them.
    poolRunning = false;
    queueCV.notify_all();
    for (auto& thread : workerThreads) {
        thread.join();
    }
    workerThreads.clear();
}

Server::~Server() {
    if (is_running || poolRunning) {
        this->Disconnect();
    }
}
