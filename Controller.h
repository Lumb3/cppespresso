/**
* @file Controller.h
 * @brief Controller class declaration (listens for user input)
 * @author Erkhembileg Ariunbold
 * @date 2026-04-24
 */


#ifndef CPPESPRESSO_CONTROLLER_H
#define CPPESPRESSO_CONTROLLER_H

#include <atomic>
#include <termios.h>

class Controller {

    static const int Default_port = 8080;
    static const char CTRL_R = 18; // ASCII 18 = Ctrl+R
    static termios originalTermios;


    /**
    * @brief Puts the terminal into raw mode so individual keystrokes are
    *        delivered immediately (no buffering, no echo).
    */
    static void enableRawMode();

public:

    static std::atomic <bool> shouldStop;
    static std::atomic <bool> shouldReload;

    /**
    *  @brief Runs on a background thread; watches stdin for Ctrl+R.
    *
    * Uses select() with a short timeout so the loop can notice shouldStop
    * without blocking indefinitely on read().
    */
    void keyboardListener();

    int getDefaultPort() const;

    /**
    * @brief Restores the terminal to its original settings.
    */
    static void disableRawMode();

    /**
    * @brief Listens for the Ctrl+C (shut down) signal from the keyboard
    * @param signum Signal number passed by the OS (unused)
    */
    static void shutDownController(int);
};



#endif //CPPESPRESSO_CONTROLLER_H
