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
#include <termios.h>
#include <unistd.h>

static std::atomic <bool> shouldStop{false};
static std::atomic <bool> shouldReload{false};


constexpr int Default_port = 8080;
constexpr char CTRL_R = 18; // ASCII 18 = Ctrl+R


static termios originalTermios;

/**
 * @brief Puts the terminal into raw mode so individual keystrokes are
 *        delivered immediately (no buffering, no echo).
 */
static void enableRawMode() {
    tcgetattr(STDIN_FILENO, &originalTermios);
    termios raw = originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON); // disable echo + line-buffering
    raw.c_cc[VMIN]  = 1;             // read() returns after 1 byte
    raw.c_cc[VTIME] = 0;             // no timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

/**
 * @brief Restores the terminal to its original settings.
 */
static void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
}

/**
 * @brief Listens for the Ctrl+C (shut down) signal from the keyboard
 * @param signum Signal number passed by the OS (unused)
 */
static void shutDownController(int) {
    shouldStop = true;
}

/**
 * @brief Runs on a background thread; watches stdin for Ctrl+R.
 *
 * Uses select() with a short timeout so the loop can notice shouldStop
 * without blocking indefinitely on read().
 */
static void keyboardListener() {
    enableRawMode();

    while (!shouldStop) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        // Wake up every 100 ms to re-check shouldStop
        timeval tv{0, 100'000};

        if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0) {
            char c{};
            if (read(STDIN_FILENO, &c, 1) == 1 && c == CTRL_R) {
                std::clog << "\n[CTRL+R] Reload requested...\n";
                shouldReload = true;
            }
        }
    }

    disableRawMode();
}


/**
 * @param argc number of input commands from the CLI
 * @param argv array of inputted commands from the CLI
 * @return EXIT Code of 1 for Failure and 0 for Success
 */
int main (int argc, char* argv[]) {
    // Use default port if none provided
    int port = (argc >= 2) ? std::stoi(argv[1]) : Default_port;

    std::clog << "Listening on port " << port
             << " | Ctrl+C = stop | Ctrl+R = reload\n";

    std::signal(SIGINT, shutDownController);
    std::signal(SIGTERM, shutDownController); // docket stop, systemtcl stop

    // Start the keyboard listener as a detached background thread.
    // It owns raw-mode and cleans up when shouldStop becomes true.
    std::thread kbThread(keyboardListener);
    kbThread.detach();

    try {
        while (!shouldStop) {
            shouldReload = false;
            Server server;
            std::thread serverThread([&]() { server.Connect(port); });

            // Wait until either a stop or a reload is requested
            while (!shouldStop && !shouldReload) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            server.Disconnect();
            serverThread.join();

            if (shouldReload && !shouldStop) {
                std::clog << "[Reload] Restarting server on port "
                          << port << "...\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << '\n';
        disableRawMode(); // safety net if detached thread hasn't run yet
        return 1;
    }
    return 0;
}