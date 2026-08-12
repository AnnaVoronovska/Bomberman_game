#include "Command.h"
#include "TestFactory.h"
#include "World.h"
#include <gtest/gtest.h>

using namespace bomberman;

class CommandTest : public ::testing::Test {
protected:
    TestFactory factory;
    Score score;
    World world{factory, score};

    void SetUp() override {
        std::remove("highscores.txt");
        world.generateArena();
    }
    void TearDown() override { std::remove("highscores.txt"); }
};

TEST_F(CommandTest, MoveCommandSetsPlayerDirection) {
    MoveCommand cmd(Direction::Right);
    cmd.execute(world);
    EXPECT_EQ(world.getPlayer()->getDirection(), Direction::Right);
}

TEST_F(CommandTest, MoveCommandNoneStopsThePlayer) {
    MoveCommand moveRight(Direction::Right);
    moveRight.execute(world);
    ASSERT_EQ(world.getPlayer()->getDirection(), Direction::Right);

    MoveCommand stop(Direction::None);
    stop.execute(world);
    EXPECT_EQ(world.getPlayer()->getDirection(), Direction::None);
}

TEST_F(CommandTest, MoveCommandDescribeMatchesDirection) {
    EXPECT_EQ(MoveCommand(Direction::Up).describe(), "Move(Up)");
    EXPECT_EQ(MoveCommand(Direction::Down).describe(), "Move(Down)");
    EXPECT_EQ(MoveCommand(Direction::Left).describe(), "Move(Left)");
    EXPECT_EQ(MoveCommand(Direction::Right).describe(), "Move(Right)");
    EXPECT_EQ(MoveCommand(Direction::None).describe(), "Move(None)");
}

TEST_F(CommandTest, PlaceBombCommandRequestsABombOnNextUpdate) {
    // requestPlayerBomb() zet enkel een vlag; World::update() plaatst de bom
    // effectief. Na de command + 1 update mag de speler geen tweede bom
    // meer kunnen plaatsen (maxBombs_ == 1).
    ASSERT_TRUE(world.getPlayer()->canPlaceBomb());
    PlaceBombCommand cmd;
    cmd.execute(world);
    world.update(0.1);
    EXPECT_FALSE(world.getPlayer()->canPlaceBomb());
}

TEST_F(CommandTest, PlaceBombCommandDescribe) {
    PlaceBombCommand cmd;
    EXPECT_EQ(cmd.describe(), "PlaceBomb");
}

TEST_F(CommandTest, CommandHistoryRecordsInOrder) {
    CommandHistory history;
    EXPECT_TRUE(history.empty());

    auto move = std::make_shared<MoveCommand>(Direction::Left);
    auto bomb = std::make_shared<PlaceBombCommand>();
    history.record(move);
    history.record(bomb);

    EXPECT_EQ(history.size(), 2u);
    EXPECT_EQ(history.last()->describe(), "PlaceBomb");
}

TEST_F(CommandTest, CommandHistoryClear) {
    CommandHistory history;
    history.record(std::make_shared<PlaceBombCommand>());
    ASSERT_EQ(history.size(), 1u);
    history.clear();
    EXPECT_TRUE(history.empty());
}

TEST_F(CommandTest, CommandsExecutedThroughBasePointerStillDispatchCorrectly) {
    // Klassieke polymorfie-check voor het Command-patroon: via een
    // ICommand*-pointer moet nog steeds de juiste execute() aangeroepen worden.
    std::vector<std::unique_ptr<ICommand>> commands;
    commands.push_back(std::make_unique<MoveCommand>(Direction::Down));
    commands.push_back(std::make_unique<PlaceBombCommand>());

    for (auto& c : commands)
        c->execute(world);

    EXPECT_EQ(world.getPlayer()->getDirection(), Direction::Down);
    world.update(0.1);
    EXPECT_FALSE(world.getPlayer()->canPlaceBomb());
}
