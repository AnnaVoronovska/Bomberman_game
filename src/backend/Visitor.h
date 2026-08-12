#pragma once
#include <string>

// ============================================================
// Visitor.h
// Visitor design pattern: laat je een nieuwe operatie op de hele
// EntityModel-hiërarchie (Wall, PowerUp, Door, Bomb, Character) toevoegen
// zonder die klassen zelf te moeten aanpassen. Elke concrete visitor
// implementeert wat er per entiteitstype moet gebeuren (double dispatch via
// entity.accept(visitor)).
// ============================================================

namespace bomberman {

class Wall;
class PowerUp;
class Door;
class Bomb;
class Character;

class EntityVisitor {
public:
    virtual ~EntityVisitor() = default;

    virtual void visit(Wall& wall) = 0;
    virtual void visit(PowerUp& powerUp) = 0;
    virtual void visit(Door& door) = 0;
    virtual void visit(Bomb& bomb) = 0;
    virtual void visit(Character& character) = 0;
};

// Concrete visitor: bouwt een korte, mensleesbare beschrijving van een
// entiteit op (bv. "Muur (breekbaar)", "Power-up: Fire", ...). Gebruikt door
// World::describeEntitiesAt() voor een debug-overlay/HUD-tooltip.
class EntityLabelVisitor : public EntityVisitor {
public:
    void visit(Wall& wall) override;
    void visit(PowerUp& powerUp) override;
    void visit(Door& door) override;
    void visit(Bomb& bomb) override;
    void visit(Character& character) override;

    const std::string& getLabel() const { return label_; }

private:
    std::string label_;
};

// Concrete visitor: geeft aan hoeveel score-punten je zou winnen door deze
// entiteit te vernietigen/oprapen (Score.h kent de exacte bedragen, deze
// visitor spiegelt ze puur voor UI/AI-doeleinden, bv. om bots te laten
// prioriteren welk doel het "waard" is).
class EntityScoreValueVisitor : public EntityVisitor {
public:
    void visit(Wall& wall) override;
    void visit(PowerUp& powerUp) override;
    void visit(Door& door) override;
    void visit(Bomb& bomb) override;
    void visit(Character& character) override;

    int getValue() const { return value_; }

private:
    int value_ = 0;
};

} // namespace bomberman