#pragma once
#include "Entities.h" // Direction
#include <memory>
#include <string>
#include <vector>

// ============================================================
// Command.h
// Command design pattern: een speler-actie (bewegen, bom plaatsen) wordt
// verpakt als een object i.p.v. rechtstreeks World::setPlayerDirection() /
// World::requestPlayerBomb() aan te roepen. Dit ontkoppelt de invoerbron
// (toetsenbord in Game.cpp, maar evengoed een AI-script of een replaysysteem)
// van de logica die de actie effectief uitvoert (World). Hoort bij de
// logic-library: geen SFML hier, enkel een World-forward-declaration.
// ============================================================

namespace bomberman {

class World;

// Command-interface: elke concrete command weet hoe hij zichzelf op een
// World uitvoert, en kan zichzelf beschrijven (nuttig voor logging/tests).
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute(World& world) = 0;
    virtual std::string describe() const = 0;
};

// Verandert de bewegingsrichting van de speler (Direction::None = stilstaan).
class MoveCommand : public ICommand {
public:
    explicit MoveCommand(Direction direction) : direction_(direction) {}

    void execute(World& world) override;
    std::string describe() const override;

    Direction getDirection() const { return direction_; }

private:
    Direction direction_;
};

// Vraagt aan om een bom te plaatsen op de huidige positie van de speler.
// De effectieve validatie (canPlaceBomb(), cooldown, ...) blijft in World.
class PlaceBombCommand : public ICommand {
public:
    void execute(World& world) override;
    std::string describe() const override;
};

// Houdt bij welke commands uitgevoerd zijn, in volgorde.
// geen vereiste voor het Command-patroon, maar een nuttige uitbreiding
// (bv. voor debugging, replay, of een "laatste actie"-indicator in de HUD).
class CommandHistory {
public:
    void record(const std::shared_ptr<ICommand>& command) { history_.push_back(command); }
    std::size_t size() const { return history_.size(); }
    void clear() { history_.clear(); }
    const std::shared_ptr<ICommand>& last() const { return history_.back(); }
    bool empty() const { return history_.empty(); }

private:
    std::vector<std::shared_ptr<ICommand>> history_;
};

} // namespace bomberman