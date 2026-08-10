#pragma once
#include "Core.h"
#include "Observer.h"
#include <memory>

// ============================================================
// Entities.hpp
// Alle spel-entiteiten (Model-deel van MVC). Pure logica: geen SFML,
// geen tekencode. Elke klasse erft van EntityModel (en dus van Subject),
// zodat Views zich er via het Observer-patroon op kunnen abonneren.
// ============================================================

namespace bomberman {

// Hoeveel tijd (in seconden) een bot minimaal moet wachten tussen twee
// bom-plaatsingen. Losstaand van maxBombs_/bombsInPlay_ (die tellen enkel
// hoeveel bommen tegelijk "in de lucht" mogen zijn), dit is puur een
// tempo-rem zodat bots niet elke tick opnieuw een bom neerzetten zodra
// hun vorige bom ontploft is.
constexpr double BOT_BOMB_COOLDOWN_SECONDS = 3.0;

enum class Direction { Up, Down, Left, Right, None };

class EntityModel : public Subject {
public:
    EntityModel(Vec2 position, Vec2 size);
    virtual ~EntityModel() = default;

    virtual void update(double deltaTime) { (void)deltaTime; }

    const Vec2& getPosition() const { return position_; }
    void setPosition(const Vec2& pos) { position_ = pos; }
    const Vec2& getSize() const { return size_; }

    // Botsingsdetectie a.d.h.v. simpele overlappende rechthoeken (AABB).
    // Geen SFML-utilities: dit hoort bij de logic-library.
    bool intersects(const EntityModel& other) const;

    bool isMarkedForRemoval() const { return markedForRemoval_; }
    void markForRemoval();

protected:
    Vec2 position_;
    Vec2 size_;
    bool markedForRemoval_ = false;
};

class Wall : public EntityModel {
public:
    Wall(Vec2 position, Vec2 size, bool destructible);
    bool isDestructible() const { return destructible_; }
    void destroy(); // notify + markeren voor verwijdering

private:
    bool destructible_;
};

enum class PowerUpType { Fire, ExtraBomb, Skates, Poison, Star, Shield, Curse, Slow, Freeze, Skull };

class PowerUp : public EntityModel {
public:
    PowerUp(Vec2 position, Vec2 size, PowerUpType type);
    PowerUpType getType() const { return type_; }
    void collect(); // notify + markeren voor verwijdering

private:
    PowerUpType type_;
};

class Character; // forward declaration: Bomb heeft enkel een niet-eigenaar referentie nodig

class Bomb : public EntityModel {
public:
    Bomb(Vec2 position, Vec2 size, int radius, std::weak_ptr<Character> owner, double fuseTime = 2.0);

    void update(double deltaTime) override; // telt de lont af
    int getRadius() const { return radius_; }
    bool hasExploded() const { return exploded_; }
    std::weak_ptr<Character> getOwner() const { return owner_; }

    void explode(); // markeert de bom als ontploft (visueel/logisch signaal)
    double getTimer() const { return timer_; }
    double getFuseTime() const { return fuseTime_; }

    // Voorkomt dat een kettingreactie dezelfde bom twee keer schade laat
    // toebrengen: geeft exact één keer true terug.
    bool consumeDamageFlag();

private:
    int radius_;
    double fuseTime_;
    double timer_ = 0.0;
    bool exploded_ = false;
    bool damageApplied_ = false;
    std::weak_ptr<Character> owner_; // bom is geen eigenaar van zijn plaatser
};

class Character : public EntityModel {
public:
    Character(Vec2 position, Vec2 size, bool isBot);

    void update(double deltaTime) override; // beweging zelf gebeurt in World (botsing!)

    void setDirection(Direction dir) { direction_ = dir; }
    Direction getDirection() const { return direction_; }

    double getSpeed() const { return speed_; }
    int getMaxBombs() const { return maxBombs_; }
    int getBombRadius() const { return bombRadius_; }
    int getBombsInPlay() const { return bombsInPlay_; }
    void onBombPlaced() {
        ++bombsInPlay_;
        if (isBot_) bombCooldownRemaining_ = BOT_BOMB_COOLDOWN_SECONDS;
    }
    void onBombResolved() { if (bombsInPlay_ > 0) --bombsInPlay_; }
    bool canPlaceBomb() const {
        return alive_ && bombsInPlay_ < maxBombs_ && bombCooldownRemaining_ <= 0.0;
    }

    void applyPowerUp(PowerUpType type);

    bool isAlive() const { return alive_; }
    void die(); // notify + character stopt met bewegen/tekenen

    // Levens-systeem: contact met een vijand of een explosie kost 1 leven
    // i.p.v. onmiddellijk te sterven. Na een treffer is het character
    // eventjes onkwetsbaar zodat je niet meerdere levens in 1 frame verliest.
    int getLives() const { return lives_; }
    void loseLife(); // -1 leven (notify); bij 0 levens -> die()
    bool isInvulnerable() const { return invulnerableTimer_ > 0.0; }

    bool isBot() const { return isBot_; }

    // De bom waar het character net op staat: die mag je nog doorlopen
    // tot je hem verlaat (klassieke Bomberman-regel).
    std::weak_ptr<Bomb> getStandingOnBomb() const { return standingOnBomb_; }
    void setStandingOnBomb(std::weak_ptr<Bomb> bomb) { standingOnBomb_ = bomb; }

private:
    Direction direction_ = Direction::Down;
    double speed_ = 0.5;      // wereld-eenheden per seconde
    int maxBombs_ = 1;
    int bombRadius_ = 1;
    int bombsInPlay_ = 0;
    double bombCooldownRemaining_ = 0.0; // enkel relevant voor bots, zie BOT_BOMB_COOLDOWN_SECONDS
    bool alive_ = true;
    bool isBot_;
    std::weak_ptr<Bomb> standingOnBomb_;
    int lives_ = 3;
    double invulnerableTimer_ = 0.0; // seconden onkwetsbaar na een treffer
};

} // namespace bomberman