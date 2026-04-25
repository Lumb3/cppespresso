/**
* @file main.cpp
 * @brief  Executable main file
 * @author Erkhembileg Ariunbold
 * @date: 2026.04.11
 */

#include <iostream>
#include "Server.h"
#include <csignal>
#include "Controller.h"
#include <thread>
#include <chrono>


/**
 * @param argc number of input commands from the CLI
 * @param argv array of inputted commands from the CLI
 * @return EXIT Code of 1 for Failure and 0 for Success
 */
int main (int argc, char* argv[]) {
    Controller control;
    // Use default port if none provided
    int port = (argc >= 2) ? std::stoi(argv[1]) : control.getDefaultPort();

    std::clog << "Listening on port " << port
             << " | Ctrl+C = stop | Ctrl+R = reload\n";

    std::signal(SIGINT, Controller::shutDownController);
    std::signal(SIGTERM, Controller::shutDownController); // docket stop, systemtcl stop

    // Start the keyboard listener as a detached background thread.
    // It owns raw-mode and cleans up when shouldStop becomes true.
    std::thread kbThread(&Controller::keyboardListener, &control);
    kbThread.detach();

    try {
        while (!Controller::shouldStop) {
            Controller::shouldReload = false;
            Server server;
            std::thread serverThread([&]() { server.Connect(port); });

            // Wait until either a stop or a reload is requested
            while (!Controller::shouldStop && !Controller::shouldReload) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1100));
            }

            server.Disconnect();
            serverThread.join(); // blocks the main thread to continue moving on

            if (Controller::shouldReload && !Controller::shouldStop) {
                std::clog << "[Reload] Restarting server on port "
                          << port << "...\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << '\n';
        control.disableRawMode(); // safety net if detached thread hasn't run yet
        return 1;
    }
    return 0;
}