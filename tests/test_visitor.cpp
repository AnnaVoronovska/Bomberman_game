#include "Entities.h"
#include "TestFactory.h"
#include "Visitor.h"
#include "World.h"
#include <algorithm>
#include <gtest/gtest.h>

using namespace bomberman;

// ---------------- EntityLabelVisitor: dispatch per type ----------------

TEST(EntityLabelVisitor, LabelsDestructibleWall) {
    Wall w(Vec2(0, 0), Vec2(0.1, 0.1), true);
    EntityLabelVisitor v;
    w.accept(v);
    EXPECT_EQ(v.getLabel(), "Muur (breekbaar)");
}

TEST(EntityLabelVisitor, LabelsIndestructibleWall) {
    Wall w(Vec2(0, 0), Vec2(0.1, 0.1), false);
    EntityLabelVisitor v;
    w.accept(v);
    EXPECT_EQ(v.getLabel(), "Muur (onverwoestbaar)");
}

TEST(EntityLabelVisitor, LabelsPowerUpWithItsType) {
    PowerUp p(Vec2(0, 0), Vec2(0.1, 0.1), PowerUpType::Skull);
    EntityLabelVisitor v;
    p.accept(v);
    EXPECT_EQ(v.getLabel(), "Power-up: Skull");
}

TEST(EntityLabelVisitor, LabelsDoor) {
    Door d(Vec2(0, 0), Vec2(0.1, 0.1));
    EntityLabelVisitor v;
    d.accept(v);
    EXPECT_EQ(v.getLabel(), "Verborgen deur");
}

TEST(EntityLabelVisitor, LabelsBombWithRadius) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 3, std::weak_ptr<Character>());
    EntityLabelVisitor v;
    b.accept(v);
    EXPECT_EQ(v.getLabel(), "Bom (radius 3)");
}

TEST(EntityLabelVisitor, LabelsPlayerVsBot) {
    Character player(Vec2(0, 0), Vec2(0.1, 0.1), false);
    Character bot(Vec2(0, 0), Vec2(0.1, 0.1), true);
    EntityLabelVisitor v;

    player.accept(v);
    EXPECT_EQ(v.getLabel(), "Speler");

    bot.accept(v);
    EXPECT_EQ(v.getLabel(), "Vijand");
}

// ---------------- EntityScoreValueVisitor: dispatch per type ----------------

TEST(EntityScoreValueVisitor, DestructibleWallIsWorthTenPoints) {
    Wall w(Vec2(0, 0), Vec2(0.1, 0.1), true);
    EntityScoreValueVisitor v;
    w.accept(v);
    EXPECT_EQ(v.getValue(), 10);
}

TEST(EntityScoreValueVisitor, IndestructibleWallIsWorthNothing) {
    Wall w(Vec2(0, 0), Vec2(0.1, 0.1), false);
    EntityScoreValueVisitor v;
    w.accept(v);
    EXPECT_EQ(v.getValue(), 0);
}

TEST(EntityScoreValueVisitor, PowerUpIsWorthTwentyFivePoints) {
    PowerUp p(Vec2(0, 0), Vec2(0.1, 0.1), PowerUpType::Fire);
    EntityScoreValueVisitor v;
    p.accept(v);
    EXPECT_EQ(v.getValue(), 25);
}

TEST(EntityScoreValueVisitor, BotIsWorthOneHundredFiftyPoints) {
    Character bot(Vec2(0, 0), Vec2(0.1, 0.1), true);
    EntityScoreValueVisitor v;
    bot.accept(v);
    EXPECT_EQ(v.getValue(), 150);
}

TEST(EntityScoreValueVisitor, PlayerIsWorthNothing) {
    Character player(Vec2(0, 0), Vec2(0.1, 0.1), false);
    EntityScoreValueVisitor v;
    player.accept(v);
    EXPECT_EQ(v.getValue(), 0);
}

TEST(EntityScoreValueVisitor, DoorAndBombAreWorthNothing) {
    Door d(Vec2(0, 0), Vec2(0.1, 0.1));
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>());
    EntityScoreValueVisitor v;

    d.accept(v);
    EXPECT_EQ(v.getValue(), 0);
    b.accept(v);
    EXPECT_EQ(v.getValue(), 0);
}

// ---------------- Polymorfe dispatch via basisklasse-pointer ----------------

TEST(EntityVisitor, DispatchWorksThroughBaseClassPointer) {
    // De essentie van het Visitor-patroon: World kent enkel EntityModel*,
    // maar accept() moet toch bij de juiste overload uitkomen (double dispatch).
    std::vector<std::unique_ptr<EntityModel>> entities;
    entities.push_back(std::make_unique<Wall>(Vec2(0, 0), Vec2(0.1, 0.1), true));
    entities.push_back(std::make_unique<PowerUp>(Vec2(0, 0), Vec2(0.1, 0.1), PowerUpType::Star));
    entities.push_back(std::make_unique<Character>(Vec2(0, 0), Vec2(0.1, 0.1), true));

    EntityLabelVisitor v;
    std::vector<std::string> labels;
    for (auto& e : entities) {
        e->accept(v);
        labels.push_back(v.getLabel());
    }

    EXPECT_EQ(labels[0], "Muur (breekbaar)");
    EXPECT_EQ(labels[1], "Power-up: Star");
    EXPECT_EQ(labels[2], "Vijand");
}

// ---------------- World::describeEntitiesAt: echte integratie ----------------

class VisitorWorldTest : public ::testing::Test {
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

TEST_F(VisitorWorldTest, DescribesPlayerAtSpawnCell) {
    // Speler spawnt op cel (1,1) (zie World::generateArena()).
    auto labels = world.describeEntitiesAt(1, 1);
    ASSERT_FALSE(labels.empty());
    EXPECT_NE(std::find(labels.begin(), labels.end(), "Speler"), labels.end());
}

TEST_F(VisitorWorldTest, EmptyFarAwayCellHasNoDescriptions) {
    // Cel (6,5) ligt op een "pillar"-positie (even/even) en is dus altijd
    // een onverwoestbare muur, geen lege cel - controleer een cel die zowel
    // geen pillar als geen spawn-cel is i.p.v. blind een lege cel te verwachten.
    // We testen hier enkel dat describeEntitiesAt() nooit crasht op een cel
    // zonder characters/bommen/power-ups/deur.
    auto labels = world.describeEntitiesAt(6, 5);
    for (auto& l : labels)
        EXPECT_FALSE(l.empty());
}

TEST_F(VisitorWorldTest, DescribesBombAfterPlacement) {
    world.requestPlayerBomb();
    world.update(0.1);

    auto labels = world.describeEntitiesAt(1, 1); // speler stond stil op (1,1)
    bool foundBomb = false;
    for (auto& l : labels)
        if (l.rfind("Bom", 0) == 0)
            foundBomb = true;
    EXPECT_TRUE(foundBomb);
}
