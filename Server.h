/** @brief  Server class declaration
* @author Erkhembileg Ariunbold
* @date: 2026.04.11
*/

#ifndef CPPESPRESSO_SERVER_H
#define CPPESPRESSO_SERVER_H
#include <atomic>

/// @brief Amount of memory allocated to the server.
constexpr int Buffer_size = 4096;

class Server {
    ///
    std::atomic<bool> is_running;
    /// Default inactive server state (socket)
    int serverSocket = -1;
public:
    /**
     *
     */
    Server() = default;

    /**
     *
     * @param port
     */
    void Connect(int port);

    /**
     *
     */
    void Disconnect();
};


#endif //CPPESPRESSO_SERVER_H