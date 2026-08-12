#pragma once
#include "AudioManager.h"
#include "Command.h"
#include "ConcreteFactory.h"
#include "Score.h"
#include "World.h"
#include <SFML/Graphics.hpp>
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
    std::shared_ptr<AudioManager> audio_; // 1x aangemaakt, blijft over meerdere games/levels heen bestaan
    std::unique_ptr<ConcreteFactory> factory_;
    std::shared_ptr<bomberman::Score> score_; // shared_ptr nodig: Subject::attach vereist shared_ptr<Observer>
    std::unique_ptr<bomberman::World> world_;
    bomberman::CommandHistory commandHistory_; // Command-patroon: logt elke uitgevoerde actie
    State state_ = State::StartScreen;
    sf::Font font_;
    bool fontLoaded_ = false;
    sf::Texture spriteSheet_;

    // "STAGE 1"-banner die kort getoond wordt net na het starten van een level.
    double stageBannerTimer_ = 0.0;
    static constexpr double STAGE_BANNER_DURATION = 1.6;
};