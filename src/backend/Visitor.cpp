#include "Visitor.h"
#include "Entities.h"

namespace bomberman {

// ---------------- EntityLabelVisitor ----------------

void EntityLabelVisitor::visit(Wall& wall) {
    label_ = wall.isDestructible() ? "Muur (breekbaar)" : "Muur (onverwoestbaar)";
}

void EntityLabelVisitor::visit(PowerUp& powerUp) {
    static const char* names[] = {"Fire",  "ExtraBomb", "Skates", "Poison", "Star",
                                   "Shield", "Curse",     "Slow",   "Freeze", "Skull"};
    int idx = static_cast<int>(powerUp.getType());
    label_ = std::string("Power-up: ") + (idx >= 0 && idx < 10 ? names[idx] : "?");
}

void EntityLabelVisitor::visit(Door& /*door*/) { label_ = "Verborgen deur"; }

void EntityLabelVisitor::visit(Bomb& bomb) {
    label_ = "Bom (radius " + std::to_string(bomb.getRadius()) + ")";
}

void EntityLabelVisitor::visit(Character& character) {
    label_ = character.isBot() ? "Vijand" : "Speler";
}

// ---------------- EntityScoreValueVisitor ----------------
// Waarden gespiegeld van Score::onNotify() (zie Score.cpp).

void EntityScoreValueVisitor::visit(Wall& wall) { value_ = wall.isDestructible() ? 10 : 0; }

void EntityScoreValueVisitor::visit(PowerUp& /*powerUp*/) { value_ = 25; }

void EntityScoreValueVisitor::visit(Door& /*door*/) { value_ = 0; }

void EntityScoreValueVisitor::visit(Bomb& /*bomb*/) { value_ = 0; }

void EntityScoreValueVisitor::visit(Character& character) { value_ = character.isBot() ? 150 : 0; }

} // namespace bomberman
