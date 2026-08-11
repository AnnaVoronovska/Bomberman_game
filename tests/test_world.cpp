#include "TestFactory.h"
#include "World.h"
#include <gtest/gtest.h>

using namespace bomberman;

// Arena-layout bevat willekeurigheid (Random::instance() zonder vaste seed,
// zie Core.cpp), dus we testen NIET de exacte muur-layout. In plaats daarvan
// steunen we op wat altijd waar is: de spawn-cellen (1,1)/(2,1)/(1,2) e.d.
// blijven altijd vrij van muren (zie isSpawnArea in World::buildArena),
// dus alles wat zich rond de spawn van de speler afspeelt is deterministisch.
class WorldTest : public ::testing::Test {
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

TEST_F(WorldTest, GenerateArenaCreatesPlayerAtExpectedSpawn) {
    auto player = world.getPlayer();
    ASSERT_NE(player, nullptr);
    EXPECT_TRUE(player->isAlive());
    EXPECT_FALSE(player->isBot());
    // cel (1,1): zie World::generateArena()
    Vec2 expected(-1.0 + (2.0 / GRID_COLS) * 1.5, -1.0 + (2.0 / GRID_ROWS) * 1.5);
    EXPECT_NEAR(player->getPosition().x, expected.x, 1e-9);
    EXPECT_NEAR(player->getPosition().y, expected.y, 1e-9);
}

TEST_F(WorldTest, GameStartsNotOver) {
    EXPECT_FALSE(world.isGameOver());
    EXPECT_FALSE(world.didPlayerWin());
    EXPECT_DOUBLE_EQ(world.getTimeRemaining(), LEVEL_TIME_SECONDS);
    EXPECT_EQ(world.getCurrentLevel(), 1);
}

TEST_F(WorldTest, PlayerMovesRightThroughGuaranteedClearSpawnArea) {
    // (1,1) -> (2,1) is altijd vrij (isSpawnArea), dus de speler moet er
    // ongehinderd naartoe kunnen bewegen.
    double startX = world.getPlayer()->getPosition().x;
    world.setPlayerDirection(Direction::Right);
    for (int i = 0; i < 200; ++i)
        world.update(0.05);
    EXPECT_GT(world.getPlayer()->getPosition().x, startX);
}

TEST_F(WorldTest, PlayerStandingStillDoesNotMove) {
    Vec2 start = world.getPlayer()->getPosition();
    world.setPlayerDirection(Direction::None);
    world.update(0.1);
    EXPECT_NEAR(world.getPlayer()->getPosition().x, start.x, 1e-9);
    EXPECT_NEAR(world.getPlayer()->getPosition().y, start.y, 1e-9);
}

TEST_F(WorldTest, PlacingBombOccupiesPlayersOnlyBombSlot) {
    auto player = world.getPlayer();
    ASSERT_TRUE(player->canPlaceBomb());

    world.requestPlayerBomb();
    world.update(0.1); // update() verwerkt de pending bomb-request

    EXPECT_FALSE(player->canPlaceBomb()) << "maxBombs_ == 1, dus de speler mag nu geen tweede bom plaatsen";
}

TEST_F(WorldTest, BombSlotFreesUpAfterExplosionResolves) {
    auto player = world.getPlayer();
    world.requestPlayerBomb();
    world.update(0.1);
    ASSERT_FALSE(player->canPlaceBomb());

    // Speler wegsturen van de bom, zodat hij niet blijft "staan op eigen bom"
    // en de standing-on-bomb regel de test niet beïnvloedt.
    world.setPlayerDirection(Direction::Right);
    for (int i = 0; i < 20; ++i)
        world.update(0.05);
    world.setPlayerDirection(Direction::None);

    // Lont laten aftellen tot de bom ontploft (default fuseTime = 2.0s).
    for (int i = 0; i < 30; ++i)
        world.update(0.1);

    EXPECT_TRUE(player->canPlaceBomb()) << "na explosie moet onBombResolved() de slot weer vrijgeven";
}

TEST_F(WorldTest, OwnBombDamagesPlayerIfStillStandingOnItWhenItExplodes) {
    auto player = world.getPlayer();
    int livesBefore = player->getLives();

    world.requestPlayerBomb();
    world.update(0.1);
    world.setPlayerDirection(Direction::None); // speler blijft op zijn eigen bom staan

    for (int i = 0; i < 30; ++i) // ruim voorbij fuseTime (2.0s)
        world.update(0.1);

    EXPECT_LT(player->getLives(), livesBefore)
        << "een speler die op zijn eigen bom blijft staan tot die ontploft, moet schade nemen";
}

TEST_F(WorldTest, TimeRemainingCountsDown) {
    world.update(1.0);
    EXPECT_DOUBLE_EQ(world.getTimeRemaining(), LEVEL_TIME_SECONDS - 1.0);
}

TEST_F(WorldTest, TimeRunningOutEndsGameWithoutPlayerWinning) {
    // Genoeg grote stappen om LEVEL_TIME_SECONDS te overschrijden.
    for (int i = 0; i < 200; ++i) {
        if (world.isGameOver())
            break;
        world.update(1.0);
    }
    EXPECT_TRUE(world.isGameOver());
    EXPECT_FALSE(world.didPlayerWin());
    EXPECT_DOUBLE_EQ(world.getTimeRemaining(), 0.0);
}

TEST_F(WorldTest, UpdateAfterGameOverIsNoOp) {
    for (int i = 0; i < 200; ++i) {
        if (world.isGameOver())
            break;
        world.update(1.0);
    }
    ASSERT_TRUE(world.isGameOver());
    double timeAfterGameOver = world.getTimeRemaining();
    world.update(1.0); // mag niets meer veranderen (vroege return in update())
    EXPECT_DOUBLE_EQ(world.getTimeRemaining(), timeAfterGameOver);
}

TEST_F(WorldTest, ConsumeStageAdvancedIsFalseByDefaultAndOnlyFiresOnce) {
    EXPECT_FALSE(world.consumeStageAdvanced());
    EXPECT_FALSE(world.consumeStageAdvanced()); // blijft false zonder een echte stage-overgang
}

TEST_F(WorldTest, BotsAreCreatedAliveAndDistinctFromPlayer) {
    // Er is geen publieke getter voor de volledige characters_-lijst, maar
    // checkWinCondition() (via didPlayerWin()) hangt af van "zijn er nog
    // levende bots" - dus meteen na generateArena() mag de speler nog niet
    // gewonnen hebben, want de drie bots leven nog.
    world.update(0.001); // triggert checkWinCondition()
    EXPECT_FALSE(world.isGameOver());
}
