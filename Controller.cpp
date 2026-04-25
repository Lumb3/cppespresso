/**
* @file Controller.cpp
 * @brief Controller class implementation (listens for user input)
 * @author Erkhembileg Ariunbold
 * @date 2026-04-24
 */

#include "Controller.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>

std::atomic<bool> Controller::shouldStop{false};
std::atomic<bool> Controller::shouldReload{false};
termios Controller::originalTermios{};

int Controller::getDefaultPort() const {
    return this->Default_port;
}

void Controller::enableRawMode() {
    tcgetattr(STDIN_FILENO, &originalTermios);
    termios raw = originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON); // disable echo + line-buffering
    raw.c_cc[VMIN]  = 1;             // read() returns after 1 byte
    raw.c_cc[VTIME] = 0;             // no timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void Controller::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
}

void Controller::keyboardListener() {
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

void Controller::shutDownController(int) {
    shouldStop = true;
}
