#pragma once
#include "AudioManager.h"
#include "Command.h"
#include "ConcreteFactory.h"
#include "Score.h"
#include "World.h"
#include <SFML/Graphics.hpp>
#include <memory>

// ============================================================
// Game.h
// Representatie-laag: bezit het SFML-venster, draait de hoofdlus en
// vertaalt toetsenbordinput naar acties in de World. Bevat zelf GEEN
// spelregels - dat hoort allemaal bij World.
// ============================================================

/**
 * @brief Representatie-laag: bezit het SFML-venster, draait de hoofdlus en
 * vertaalt toetsenbordinput naar acties in de World. Bevat zelf GEEN
 * spelregels - dat hoort allemaal bij World.
 */
class Game {
public:
    Game();
    /// @brief Start en draait de volledige hoofdlus van het spel tot het venster sluit.
    void run();

private:
    enum class State { StartScreen, Playing, GameOver };

    /// @brief Verwerkt SFML-events (toetsenbord, venster sluiten, ...) voor deze frame.
    void processInput();
    /// @brief Werkt de spellogica (World) en timers bij met het gegeven deltaTime.
    void update(double dt);
    /// @brief Tekent het huidige frame (arena, HUD, overlays, ...) op het venster.
    void render(double dt);
    /// @brief Zet World/Score/Factory terug in beginstaat voor een nieuwe game.
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

    // F1 toggelt een debug-overlay die via het Visitor-patroon
    // (World::describeEntitiesAt) toont welke entiteiten er op de cel van de
    // speler staan. Puur ontwikkelaars-hulpmiddel, geen invloed op gameplay.
    bool showDebugOverlay_ = false;
};