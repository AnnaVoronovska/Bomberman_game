#pragma once
#include "Core.h"
#include "Observer.h"
#include "Entities.h"
#include "AbstractFactory.h"
#include "Score.h"
#include <vector>
#include <memory>
#include <utility>

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

class World : public Subject {
public:
    World(AbstractFactory& factory, Score& score);

    void generateArena();          // bouwt de arena volgens de klassieke layout
    void update(double deltaTime);

    bool isGameOver() const { return gameOver_; }
    bool didPlayerWin() const { return playerWon_; }
    std::shared_ptr<Character> getPlayer() const { return player_; }

    // Input van de representatie-laag wordt hier vertaald naar logica.
    void setPlayerDirection(Direction dir);
    void requestPlayerBomb();

private:
    Vec2 cellToWorld(int col, int row) const;
    Wall* wallAtCell(int col, int row) const;
    bool isBombAt(int col, int row) const;

    void tryMoveCharacter(Character& c, double deltaTime);
    void placeBomb(const std::shared_ptr<Character>& owner);
    void updateBombs(double deltaTime);
    void explodeBomb(Bomb& bomb);
    void spreadExplosion(int col, int row, int dcol, int drow, int radius,
                          std::vector<std::pair<int, int>>& affected);
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

    std::shared_ptr<Character> player_;

    bool gameOver_ = false;
    bool playerWon_ = false;
    bool pendingPlayerBomb_ = false;

    void collectDangerTiles(std::vector<std::vector<bool>>& danger) const;
    bool findEscapeDirection(int fromCol, int fromRow,
                              const std::vector<std::vector<bool>>& danger,
                              Direction& outDir) const;
};

} // namespace bomberman
