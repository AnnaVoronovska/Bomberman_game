#include "Entities.h"
#include <algorithm>

namespace bomberman {

// ---------------- EntityModel ----------------

EntityModel::EntityModel(Vec2 position, Vec2 size)
    : position_(position), size_(size) {}

bool EntityModel::intersects(const EntityModel& other) const {
    double aLeft = position_.x - size_.x / 2.0;
    double aRight = position_.x + size_.x / 2.0;
    double aTop = position_.y - size_.y / 2.0;
    double aBottom = position_.y + size_.y / 2.0;

    double bLeft = other.position_.x - other.size_.x / 2.0;
    double bRight = other.position_.x + other.size_.x / 2.0;
    double bTop = other.position_.y - other.size_.y / 2.0;
    double bBottom = other.position_.y + other.size_.y / 2.0;

    return aLeft < bRight && aRight > bLeft && aTop < bBottom && aBottom > bTop;
}

void EntityModel::markForRemoval() {
    markedForRemoval_ = true;
    notify(Event{EventType::Removed, this});
}

// ---------------- Wall ----------------

Wall::Wall(Vec2 position, Vec2 size, bool destructible)
    : EntityModel(position, size), destructible_(destructible) {}

void Wall::destroy() {
    notify(Event{EventType::BlockDestroyed, this});
    markForRemoval();
}

// ---------------- PowerUp ----------------

PowerUp::PowerUp(Vec2 position, Vec2 size, PowerUpType type)
    : EntityModel(position, size), type_(type) {}

void PowerUp::collect() {
    notify(Event{EventType::PowerUpCollected, this});
    markForRemoval();
}

// ---------------- Bomb ----------------

Bomb::Bomb(Vec2 position, Vec2 size, int radius, std::weak_ptr<Character> owner, double fuseTime)
    : EntityModel(position, size), radius_(radius), fuseTime_(fuseTime), owner_(owner) {}

void Bomb::update(double deltaTime) {
    if (exploded_) return;
    timer_ += deltaTime;
    if (timer_ >= fuseTime_) {
        explode();
    }
}

void Bomb::explode() {
    if (exploded_) return;
    exploded_ = true;
    notify(Event{EventType::BombExploded, this});
    markForRemoval();
}

bool Bomb::consumeDamageFlag() {
    if (damageApplied_) return false;
    damageApplied_ = true;
    return true;
}

// ---------------- Character ----------------

Character::Character(Vec2 position, Vec2 size, bool isBot)
    : EntityModel(position, size), isBot_(isBot) {}

void Character::update(double deltaTime) {
    // Beweging gebeurt bewust NIET hier: World moet eerst botsingen met
    // muren en bommen controleren voor de positie effectief verandert.
    (void)deltaTime;
}

void Character::applyPowerUp(PowerUpType type) {
    switch (type) {
        case PowerUpType::Fire:      ++bombRadius_; break;
        case PowerUpType::ExtraBomb: ++maxBombs_;   break;
        case PowerUpType::Skates:    speed_ += 0.15; break;
        case PowerUpType::Poison:    speed_ = std::max(0.1, speed_ - 0.1); break;
        case PowerUpType::Star:      /* score-bonus wordt via Score/Event afgehandeld */ break;
        case PowerUpType::Shield:    bombRadius_ += 2; break;
        case PowerUpType::Curse:     maxBombs_ = std::max(1, maxBombs_ - 1); break;
        case PowerUpType::Slow:      speed_ = std::max(0.1, speed_ - 0.15); break;
        case PowerUpType::Freeze:    speed_ += 0.25; break;
        case PowerUpType::Skull:     bombRadius_ = std::max(1, bombRadius_ - 1); break;
    }
}

void Character::die() {
    if (!alive_) return;
    alive_ = false;
    notify(Event{EventType::Died, this});
}

} // namespace bomberman