#pragma once
#include "Core.h"
#include "Entities.h"
#include <memory>

// ============================================================
// AbstractFactory.hpp
// Abstract Factory design pattern: World kent enkel deze interface en
// weet dus NIETS over SFML. De representatie-laag levert een
// ConcreteFactory die bovendien meteen de juiste View (Observer)
// koppelt aan elke aangemaakte entity.
// ============================================================

namespace bomberman {

    class AbstractFactory {
    public:
        virtual ~AbstractFactory() = default;

        virtual std::shared_ptr<Character> createCharacter(Vec2 position, bool isBot) = 0;
        virtual std::shared_ptr<Wall> createWall(Vec2 position, bool destructible) = 0;
        virtual std::shared_ptr<Bomb> createBomb(Vec2 position, int radius, std::weak_ptr<Character> owner) = 0;
        virtual std::shared_ptr<PowerUp> createPowerUp(Vec2 position, PowerUpType type) = 0;
        virtual std::shared_ptr<Door> createDoor(Vec2 position) = 0;
    };

} // namespace bomberman