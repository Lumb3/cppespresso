/**
* @file main.cpp
 * @brief  Executable main file
 * @author Erkhembileg Ariunbold
 * @date: 2026.04.11
 */

#include <iostream>
#include "Server.h"
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

static std::atomic <bool> shouldStop{false};

constexpr int Default_port = 8080;

/**
 * @brief Listens for the Ctrl+C (shut down) signal from the keyboard
 * @param signum Signal number passed by the OS (unused)
 */
static void shutDownController(int) {
    shouldStop = true;
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
        std::signal(SIGINT, shutDownController);
        std::signal(SIGTERM, shutDownController); // docket stop, systemtcl stop

        // Run Connect() on a background thread
        std::thread serverThread([&]() {server.Connect(port);});

        while (!shouldStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        server.Disconnect();
        serverThread.join(); // blocks the main thread to continue moving on

    } catch (const std::exception& e) {
        std::cout << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}