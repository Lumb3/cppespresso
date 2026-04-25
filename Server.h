/**
 * @file Server.h
 * @brief Declaration of the Server class for handling HTTP server operations.
 * @author Erkhembileg Ariunbold
 * @date 2026-04-11
 */

#ifndef CPPESPRESSO_SERVER_H
#define CPPESPRESSO_SERVER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

/// @brief Size (in bytes) of the read buffer used per client connection.
constexpr int Buffer_size = 4096;

/**
 *
 */
class Server {

    // -------------------------------------------------------------------------
    // Thread pool
    // -------------------------------------------------------------------------

    /// @brief Fixed number of worker threads in the pool.
    static constexpr int THREAD_POOL_SIZE = 8;

    /// @brief Worker threads — created in Connect(), joined in Disconnect().
    std::vector<std::thread> workerThreads;

    /// @brief Queue of accepted client socket file descriptors awaiting handling.
    std::queue<int> taskQueue;

    /// @brief Mutex protecting taskQueue.
    std::mutex queueMutex;

    /// @brief Condition variable used to wake workers when a new socket is queued.
    std::condition_variable queueCV;

    /// @brief Set to true while the thread pool is active; false signals workers to exit.
    std::atomic<bool> poolRunning{false};

    // -------------------------------------------------------------------------
    // Server socket
    // -------------------------------------------------------------------------

    /// @brief Whether the server accept loop is running.
    std::atomic<bool> is_running{false};

    /**
     * @brief File descriptor for the listening server socket.
     *
     * Stored as an atomic int so Disconnect() can safely close it from a
     * signal handler or a separate thread without a data race.
     * Initialized to -1 to indicate no open socket.
     */
    std::atomic<int> serverSocket{-1};

    // -------------------------------------------------------------------------
    // Private methods
    // -------------------------------------------------------------------------

    /**
     * @brief Entry point for each worker thread in the pool.
     *
     * Blocks on queueCV until a client socket is available, then calls
     * HandleClient(). Exits cleanly when poolRunning is false and the
     * queue is empty.
     */
    void WorkerLoop();

    /**
     * @brief Reads a full HTTP request from clientSocket and writes a response.
     *
     * Reads in a loop until both headers and the declared Content-Length body
     * have been received, then parses and routes the request. A 5-second
     * SO_RCVTIMEO is applied so a stalled client cannot hold the thread
     * indefinitely. Closes clientSocket before returning.
     *
     * @param clientSocket File descriptor of the accepted client connection.
     */
    static void HandleClient(int clientSocket);

    /**
     *
     * @param path
     * @return
     */
    static std::string getMimeType(const std::string& path);

    // HTTP data types
    /**
     * @struct HttpRequest
     * @brief Represents a parsed inbound HTTP request.
     */
    struct HttpRequest {
        std::string method;   ///< HTTP verb (e.g. "GET", "POST").
        std::string path;     ///< Request path (e.g. "/health").
        std::string headers;  ///< Raw header block (everything before the blank line).
        std::string body;     ///< Request body (empty for GET/HEAD).
    };

    /**
     * @struct HttpResponse
     * @brief Represents an outbound HTTP response.
     *
     * Populate status, contentType, and body, then call toString() to
     * serialise to a wire-format string ready for send().
     */
    struct HttpResponse {
        int  status = 200;          /// HTTP status code.
        std::string contentType = "text/plain"; /// Default content type: plain text
        std::string body;                       /// Response body.

        /**
         * @brief Serialises the response to an HTTP/1.1 wire-format string.
         *
         * Status text is derived from status at call time (not construction
         * time) so that mutating status after construction is reflected correctly.
         *
         * @return A string of the form "HTTP/1.1 <status> <text>\r\n...".
         */
        std::string toString() const {
            const std::string statusText =
                (status == 200) ? "OK"                    :
                (status == 404) ? "Not Found"             :
                (status == 500) ? "Internal Server Error" : "Unknown";

            return "HTTP/1.1 " + std::to_string(status) + " " + statusText + "\r\n"
                   "Content-Type: "   + contentType + "\r\n"
                   "Content-Length: " + std::to_string(body.size()) + "\r\n"
                   "Connection: close\r\n\r\n" +
                   body;
        }
    };

    /**
     *
     * @param path
     * @return
     */
    static HttpResponse serveFile(const std::string& path);

    /**
     *
     * @param requestedPath
     * @return
     */
    static std::string ResolvePath(const std::string& requestedPath);
public:

    // Routing
    /**
     * @brief Parses a raw HTTP request string into an HttpRequest struct.
     *
     * Extracts the method, path, header block, and body from the raw bytes
     * received over the socket. Does not validate individual header values.
     *
     * @param raw The complete raw HTTP request string.
     * @return    A populated HttpRequest.
     */
    static HttpRequest parseRequest(const std::string& raw);

    /**
     * @brief Maps an HttpRequest to an HttpResponse based on method and path.
     *
     * Current routes:
     *  - POST /       → 200 with echoed body
     *  - GET  /health → 200 "OK"
     *  - anything else → 404 "Not Found"
     *
     * @param req The parsed request to route.
     * @return    The appropriate HttpResponse.
     */
    static HttpResponse handleRoute(const HttpRequest& req);


    // Lifecycle
    /// @brief Constructs a Server in a non-running state.
    Server() = default;

    /**
     * @brief Starts the server: creates a socket, binds to port, and blocks
     *        in the accept loop until Disconnect() is called.
     *
     * Spawns THREAD_POOL_SIZE worker threads before entering the accept loop.
     * Each accepted connection is pushed onto the task queue and handled by
     * the next available worker.
     *
     * @param port TCP port to listen on (e.g. 8080).
     * @throws std::runtime_error if socket creation, bind, or listen fails.
     */
    void Connect(int port);

    /**
     * @brief Stops the server and releases all resources.
     *
     * Closes the server socket (causing the accept loop to exit), signals
     * all worker threads to finish, and joins them. Safe to call from a
     * signal handler or a separate thread.
     */
    void Disconnect();

    /**
     *  Destructor that Disconnects server when it is running.
     */
    ~Server();
};

#endif // CPPESPRESSO_SERVER_H