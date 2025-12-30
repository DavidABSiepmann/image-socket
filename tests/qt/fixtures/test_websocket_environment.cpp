/**
 * @file test_websocket_environment.cpp
 * @brief MOC implementation for test_websocket_environment.h
 *
 * This file provides the required MOC/linking code for the
 * TestWebSocketEnvironment and WebSocketTestClient classes.
 *
 * The fixture code is mostly implemented in the header, but Qt's MOC
 * requires a corresponding .cpp file for proper linking.
 */

#include "test_websocket_environment.h"

// MOC is handled by CMake automoc; explicit inclusion of the generated .moc
// caused warnings when the translation unit does not contain Q_OBJECT. The
// header contains Q_OBJECT macros, so the moc will be generated automatically
// and compiled by CMake's automoc; no explicit #include is necessary here.
