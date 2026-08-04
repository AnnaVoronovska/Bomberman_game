#include "Game.h"
#include <iostream>
#include <exception>

// ============================================================
// main.cpp
// Enige verantwoordelijkheid: Game opstarten en onverwachte fouten
// netjes opvangen (vereiste exception handling uit sectie 3.2).
// ============================================================

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
