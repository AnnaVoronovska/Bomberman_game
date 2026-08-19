#pragma once
#include "Core.h"
#include "Entities.h"
#include <memory>

// ============================================================
// AbstractFactory.h
// Abstract Factory design pattern: World kent enkel deze interface en
// weet dus NIETS over SFML. De representatie-laag levert een
// ConcreteFactory die bovendien meteen de juiste View (Observer)
// koppelt aan elke aangemaakte entity.
// ============================================================

namespace bomberman {

    /**
     * @brief Abstract Factory interface waarmee World entities kan aanmaken
     * zonder te weten hoe hun (SFML-)View eraan gekoppeld wordt.
     */
    class AbstractFactory {
    public:
        virtual ~AbstractFactory() = default;

        /// @brief Maakt een Character aan (Player of bot) op de gegeven positie.
        virtual std::shared_ptr<Character> createCharacter(Vec2 position, bool isBot) = 0;
        /// @brief Maakt een Wall aan op de gegeven positie, breekbaar of niet.
        virtual std::shared_ptr<Wall> createWall(Vec2 position, bool destructible) = 0;
        /// @brief Maakt een Bomb aan op de gegeven positie met gegeven radius en eigenaar.
        virtual std::shared_ptr<Bomb> createBomb(Vec2 position, int radius, std::weak_ptr<Character> owner) = 0;
        /// @brief Maakt een PowerUp aan van het gegeven type op de gegeven positie.
        virtual std::shared_ptr<PowerUp> createPowerUp(Vec2 position, PowerUpType type) = 0;
        /// @brief Maakt een Door aan op de gegeven positie.
        virtual std::shared_ptr<Door> createDoor(Vec2 position) = 0;
    };

} // namespace bomberman