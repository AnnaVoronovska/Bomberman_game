#include "Command.h"
#include "World.h"

namespace bomberman {

void MoveCommand::execute(World& world) { world.setPlayerDirection(direction_); }

std::string MoveCommand::describe() const {
    switch (direction_) {
    case Direction::Up:
        return "Move(Up)";
    case Direction::Down:
        return "Move(Down)";
    case Direction::Left:
        return "Move(Left)";
    case Direction::Right:
        return "Move(Right)";
    default:
        return "Move(None)";
    }
}

void PlaceBombCommand::execute(World& world) { world.requestPlayerBomb(); }

std::string PlaceBombCommand::describe() const { return "PlaceBomb"; }

} // namespace bomberman
