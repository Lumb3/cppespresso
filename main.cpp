/**
* @file main.cpp
 * @brief  Executable main file
 * @author Erkhembileg Ariunbold
 * @date: 2026.04.11
 */

#include <iostream>
#include "Server.h"
#include <csignal>

constexpr int Default_port = 8080;
static Server *global_server = nullptr;

/**
 * @brief Listens for the Ctrl+C (shut down) signal from the keyboard
 * @param signum Signal number passed by the OS (unused)
 */
static void shutDownController(int) {
    if (global_server) {
        global_server->Disconnect();
    }
}

/**
 * @param argc number of input commands from the CLI
 * @param argv array of inputted commands from the CLI
 * @return EXIT Code of 1 for Failure and 0 for Success
 */
int main (int argc, char* argv[]) {
    // Use default port if none provided
    int port = (argc >= 2) ? std::stoi(argv[1]) : Default_port;

    std::cout << "Listening on port: " << port << "..." << std::endl;

    try {
        Server server;

        /// global_server points to the server instance on the stack,
        /// allowing shutDownController() to call Disconnect() on it.
        global_server = &server;
        std::signal(SIGINT, shutDownController);
        server.Connect(port);

    } catch (const std::exception& e) {
        std::cout << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}