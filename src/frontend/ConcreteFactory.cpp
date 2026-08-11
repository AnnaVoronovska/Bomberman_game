#include "ConcreteFactory.h"
#include "World.h" // voor GRID_COLS / GRID_ROWS
#include <algorithm>

using namespace bomberman;

ConcreteFactory::ConcreteFactory(const Camera& camera, const sf::Texture& texture)
    : camera_(camera), texture_(texture) {}

std::shared_ptr<Character> ConcreteFactory::createCharacter(Vec2 position, bool isBot) {
    auto model = std::make_shared<Character>(position, Vec2(0.11, 0.11), isBot);

    // (rowBase, colBase) per skin: down-frame linkerbovenhoek in de spritesheet.
    // up = rowBase+1, left = rowBase+2, telkens zelfde colBase.
    // speler = roze (rij 1, kolom 0).
    // vijand 1 = rood (rij 12, kolom 0), vijand 2 = blauw (rij 12, kolom 8),
    // vijand 3 = donker (rij 1, kolom 8).
    static const int enemyRow[3] = {12, 12, 1};
    static const int enemyCol[3] = {0, 8, 8};

    int rowBase, colBase;
    if (isBot) {
        int i = botIndex_++ % 3;
        rowBase = enemyRow[i];
        colBase = enemyCol[i];
    } else {
        rowBase = 1;
        colBase = 0;
    }

    auto view = std::make_shared<CharacterView>(model, camera_, texture_, rowBase, colBase);
    model->attach(view);
    views_.push_back(view);
    return model;
}

std::shared_ptr<Wall> ConcreteFactory::createWall(Vec2 position, bool destructible) {
    auto model = std::make_shared<Wall>(position, Vec2(2.0 / GRID_COLS, 2.0 / GRID_ROWS), destructible);
    auto view = std::make_shared<WallView>(model, camera_, texture_);
    model->attach(view);
    views_.push_back(view);
    return model;
}

std::shared_ptr<Bomb> ConcreteFactory::createBomb(Vec2 position, int radius, std::weak_ptr<Character> owner) {
    auto model = std::make_shared<Bomb>(position, Vec2(0.09, 0.09), radius, owner);
    auto view = std::make_shared<BombView>(model, camera_, texture_);
    model->attach(view);
    views_.push_back(view);
    return model;
}

std::shared_ptr<PowerUp> ConcreteFactory::createPowerUp(Vec2 position, PowerUpType type) {
    auto model = std::make_shared<PowerUp>(position, Vec2(0.09, 0.09),
                                           type); // iets groter dan voorheen (0.06), zodat het icoon goed zichtbaar is
    auto view = std::make_shared<PowerUpView>(model, camera_, texture_);
    model->attach(view);
    views_.push_back(view);
    return model;
}

std::shared_ptr<Door> ConcreteFactory::createDoor(Vec2 position) {
    auto model = std::make_shared<Door>(position, Vec2(2.0 / GRID_COLS, 2.0 / GRID_ROWS));
    auto view = std::make_shared<DoorView>(model, camera_);
    model->attach(view);
    views_.push_back(view);
    return model;
}

void ConcreteFactory::drawAll(sf::RenderWindow& window) {
    for (auto& v : views_) {
        if (!v->isExpired())
            v->draw(window);
    }
}

void ConcreteFactory::removeExpiredViews() {
    views_.erase(std::remove_if(views_.begin(), views_.end(),
                                [](const std::shared_ptr<EntityView>& v) { return v->isExpired(); }),
                 views_.end());
}