/**
* @file Server.h
 * @brief Declaration of the Server class for handling basic server operations.
 * @author Erkhembileg Ariunbold
 * @date 2026-04-11
 */

#ifndef CPPESPRESSO_SERVER_H
#define CPPESPRESSO_SERVER_H

#include <atomic>
#include <string>
/// @brief Size (in bytes) of the buffer used by the server.
constexpr int Buffer_size = 4096;

/**
 * @class Server
 * @brief Provides functionality to manage a simple server lifecycle.
 *
 * This class handles starting and stopping a server, as well as
 * maintaining its running state.
 */
class Server {
    /**
     * @brief Indicates whether the server is currently running.
     * Uses atomic operations to ensure thread-safe access.
     */
    std::atomic<bool> is_running{false};

    /**
     * @brief File descriptor for the server socket.
     *
     * Initialized to -1 to indicate an invalid or inactive socket.
     */
    std::atomic<int> serverSocket = -1;

    /**
     * Number of connected clients (thread count)
     */
    std::atomic<int> clientCount = 0;
    /**
    * A method for concurrently handling a new client using threads.
    *
    * @param clientSocket
    */
    void HandleClient(int clientSocket);

public:
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string headers;
        std::string body;
    };

    struct HttpResponse {
        int status = 200;
        std::string contentType = "text/plain";
        std::string body;

        std::string statusText = (status == 200) ? "OK" :
                                 (status == 404) ? "Not Found" :
                                 (status == 500) ? "Internal Server Error" : "Unknown";
        std::string toString() const {
            return "HTTP/1.1 " + std::to_string(status) + " " + statusText + "\r\n"
                   "Content-Type: " + contentType + "\r\n"
                   "Content-Length: " + std::to_string(body.size()) + "\r\n"
                   "Connection: close\r\n\r\n" +
                   body;
        }
    };

    HttpRequest parseRequest(const std::string& raw);
    HttpResponse handleRoute(const HttpRequest& req);

    /**
     * @brief Constructs a Server instance.
     * Initializes the server in a non-running state.
     */
    Server() = default;

    /**
     * @brief Starts the server and binds it to the specified port.
     * @param port The port number on which the server will listen.
     */
    void Connect(int port);

    /**
     * @brief Stops the server and releases associated resources.
     */
    void Disconnect();
};

#endif // CPPESPRESSO_SERVER_H