#pragma once
#include "AbstractFactory.h"
#include "World.h" // voor GRID_COLS / GRID_ROWS

// ============================================================
// TestFactory.h
// Minimale AbstractFactory-implementatie voor unit tests: maakt gewoon de
// pure logic-objecten aan, zonder enige View/Observer te koppelen (dat is
// precies waarom World volledig los van SFML getest kan worden).
// ============================================================

namespace bomberman {

class TestFactory : public AbstractFactory {
public:
    std::shared_ptr<Character> createCharacter(Vec2 position, bool isBot) override {
        return std::make_shared<Character>(position, Vec2(0.12, 0.12), isBot);
    }

    std::shared_ptr<Wall> createWall(Vec2 position, bool destructible) override {
        return std::make_shared<Wall>(position, Vec2(2.0 / GRID_COLS, 2.0 / GRID_ROWS), destructible);
    }

    std::shared_ptr<Bomb> createBomb(Vec2 position, int radius, std::weak_ptr<Character> owner) override {
        return std::make_shared<Bomb>(position, Vec2(0.1, 0.1), radius, owner);
    }

    std::shared_ptr<PowerUp> createPowerUp(Vec2 position, PowerUpType type) override {
        return std::make_shared<PowerUp>(position, Vec2(0.08, 0.08), type);
    }

    std::shared_ptr<Door> createDoor(Vec2 position) override {
        return std::make_shared<Door>(position, Vec2(0.1, 0.1));
    }
};

} // namespace bomberman
