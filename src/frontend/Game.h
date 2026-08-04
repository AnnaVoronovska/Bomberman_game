#pragma once
#include <SFML/Graphics.hpp>
#include "World.h"
#include "Score.h"
#include "ConcreteFactory.h"
#include <memory>

// ============================================================
// Game.hpp
// Representatie-laag: bezit het SFML-venster, draait de hoofdlus en
// vertaalt toetsenbordinput naar acties in de World. Bevat zelf GEEN
// spelregels - dat hoort allemaal bij World.
// ============================================================

class Game {
public:
    Game();
    void run();


private:
    enum class State { StartScreen, Playing, GameOver };

    void processInput();
    void update(double dt);
    void render();
    void startNewGame();

    sf::RenderWindow window_;
    bomberman::Camera camera_;
    std::unique_ptr<ConcreteFactory> factory_;
    std::shared_ptr<bomberman::Score> score_; // shared_ptr nodig: Subject::attach vereist shared_ptr<Observer>
    std::unique_ptr<bomberman::World> world_;
    State state_ = State::StartScreen;
    sf::Font font_;
    bool fontLoaded_ = false;
    sf::Texture spriteSheet_;
};
