#pragma once
#include "AbstractFactory.h"
#include "Core.h"
#include "Entities.h"
#include "Observer.h"
#include "Score.h"
#include <memory>
#include <utility>
#include <vector>

// ============================================================
// World.hpp
// World is de "Controller" uit MVC: hij bezit alle entity-Models en
// regisseert de volledige spellogica (spawnen, botsing, bom-explosies,
// eenvoudige bot-AI, win/verlies). Hoort volledig bij de logic-library:
// geen SFML hier.
// ============================================================

namespace bomberman {

constexpr int GRID_COLS = 13;
constexpr int GRID_ROWS = 11;
constexpr double LEVEL_TIME_SECONDS = 180.0; // 3 minuten per level

class World : public Subject {
public:
    World(AbstractFactory& factory, Score& score);

    void generateArena(); // bouwt de arena volgens de klassieke layout
    void update(double deltaTime);

    bool isGameOver() const { return gameOver_; }
    bool didPlayerWin() const { return playerWon_; }
    std::shared_ptr<Character> getPlayer() const { return player_; }

    // Level-timer: elk level heeft LEVEL_TIME_SECONDS. Als de tijd op is
    // voor de speler alle vijanden verslaat, verliest de speler.
    double getTimeRemaining() const { return timeRemaining_; }
    int getCurrentLevel() const { return currentLevel_; }

    // true als World net (deze update-tick) naar een volgende stage is gegaan,
    // via de verborgen deur. Geeft telkens maar één keer true terug: de
    // representatie-laag "consumeert" dit om de "STAGE X"-banner te tonen.
    bool consumeStageAdvanced();

    // Input van de representatie-laag wordt hier vertaald naar logica.
    void setPlayerDirection(Direction dir);
    void requestPlayerBomb();

private:
    Vec2 cellToWorld(int col, int row) const;
    std::pair<int, int> cellOfPosition(const Vec2& pos) const;
    Wall* wallAtCell(int col, int row) const;
    bool isBombAt(int col, int row) const;

    void buildArena();         // muren opnieuw opbouwen + nieuwe verborgen deur-tegel kiezen
    void advanceToNextLevel(); // speler bereikt de deur: nieuwe stage, zelfde speler/score

    void tryMoveCharacter(Character& c, double deltaTime);
    void placeBomb(const std::shared_ptr<Character>& owner);
    void updateBombs(double deltaTime);
    void explodeBomb(Bomb& bomb);
    void spreadExplosion(int col, int row, int dcol, int drow, int radius, std::vector<std::pair<int, int>>& affected);
    void applyExplosionDamage(const std::vector<std::pair<int, int>>& tiles, Bomb& source);
    void maybeSpawnPowerUp(int col, int row);
    void updateBotAI(const std::shared_ptr<Character>& bot, double deltaTime);
    void checkWinCondition();
    void removeDeadEntities();

    AbstractFactory& factory_;
    Score& score_;

    std::vector<std::shared_ptr<Wall>> walls_;
    std::vector<std::shared_ptr<Bomb>> bombs_;
    std::vector<std::shared_ptr<PowerUp>> powerUps_;
    std::vector<std::shared_ptr<Character>> characters_; // [0] = Player, rest = bots
    std::shared_ptr<Door> door_;                         // pas aangemaakt zodra de muur die hem verbergt ontploft

    std::shared_ptr<Character> player_;

    bool gameOver_ = false;
    bool playerWon_ = false;
    bool pendingPlayerBomb_ = false;
    double timeRemaining_ = LEVEL_TIME_SECONDS;
    int currentLevel_ = 1;
    bool stageAdvancedFlag_ = false;

    // Cel van de ÉÉN breekbare muur (per level willekeurig gekozen) die de
    // verborgen deur verbergt. -1 als er (toevallig) geen breekbare muur was.
    int doorCol_ = -1;
    int doorRow_ = -1;

    void collectDangerTiles(std::vector<std::vector<bool>>& danger) const;
    bool findEscapeDirection(int fromCol, int fromRow, const std::vector<std::vector<bool>>& danger,
                             Direction& outDir) const;

    // Pathfinding naar een willekeurig doel (power-up, muur om te bombarderen, ...).
    // 'danger' wordt behandeld als onbegaanbaar, net als muren/bommen, zodat een
    // bot NOOIT via een actieve gevarenzone naar zijn doel loopt (dat veroorzaakte
    // de bug waarbij een bot na het vluchten meteen terug de gevarenzone in werd
    // getrokken door prioriteit 2/4, bleef "pendelen", en soms in zijn eigen bom stierf).
    bool findPathDirection(int fromCol, int fromRow, int targetCol, int targetRow, bool targetAdjacent,
                           const std::vector<std::vector<bool>>& danger, Direction& outDir) const;
};

} // namespace bomberman