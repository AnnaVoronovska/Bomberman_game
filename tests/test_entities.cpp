#include "Entities.h"
#include <gtest/gtest.h>

using namespace bomberman;

// ---------------- EntityModel::intersects (AABB) ----------------

TEST(EntityModel, IntersectsOverlappingBoxes) {
    Wall a(Vec2(0.0, 0.0), Vec2(0.2, 0.2), false);
    Wall b(Vec2(0.1, 0.0), Vec2(0.2, 0.2), false);
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a)); // symmetrisch
}

TEST(EntityModel, DoesNotIntersectFarApartBoxes) {
    Wall a(Vec2(-1.0, -1.0), Vec2(0.2, 0.2), false);
    Wall b(Vec2(1.0, 1.0), Vec2(0.2, 0.2), false);
    EXPECT_FALSE(a.intersects(b));
}

TEST(EntityModel, TouchingEdgesDoNotIntersect) {
    // Randen die exact raken (geen overlap) tellen niet als botsing:
    // intersects() gebruikt strikte '<' / '>', dus een gedeelde rand is geen hit.
    Wall a(Vec2(0.0, 0.0), Vec2(0.2, 0.2), false);  // [-0.1, 0.1]
    Wall b(Vec2(0.2, 0.0), Vec2(0.2, 0.2), false);  // [0.1, 0.3]
    EXPECT_FALSE(a.intersects(b));
}

TEST(EntityModel, MarkForRemovalSetsFlag) {
    Wall w(Vec2(0, 0), Vec2(0.2, 0.2), true);
    EXPECT_FALSE(w.isMarkedForRemoval());
    w.markForRemoval();
    EXPECT_TRUE(w.isMarkedForRemoval());
}

// ---------------- Wall ----------------

TEST(Wall, DestructibleFlagStored) {
    Wall a(Vec2(0, 0), Vec2(0.2, 0.2), true);
    Wall b(Vec2(0, 0), Vec2(0.2, 0.2), false);
    EXPECT_TRUE(a.isDestructible());
    EXPECT_FALSE(b.isDestructible());
}

TEST(Wall, DestroyMarksForRemoval) {
    Wall w(Vec2(0, 0), Vec2(0.2, 0.2), true);
    w.destroy();
    EXPECT_TRUE(w.isMarkedForRemoval());
}

// ---------------- PowerUp ----------------

TEST(PowerUp, CollectMarksForRemoval) {
    PowerUp p(Vec2(0, 0), Vec2(0.1, 0.1), PowerUpType::Fire);
    EXPECT_FALSE(p.isMarkedForRemoval());
    p.collect();
    EXPECT_TRUE(p.isMarkedForRemoval());
}

TEST(PowerUp, TypeIsStored) {
    PowerUp p(Vec2(0, 0), Vec2(0.1, 0.1), PowerUpType::Skull);
    EXPECT_EQ(p.getType(), PowerUpType::Skull);
}

// ---------------- Bomb ----------------

TEST(Bomb, DoesNotExplodeBeforeFuseTime) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>(), /*fuseTime=*/2.0);
    b.update(1.0);
    EXPECT_FALSE(b.hasExploded());
    EXPECT_DOUBLE_EQ(b.getTimer(), 1.0);
}

TEST(Bomb, ExplodesExactlyAtFuseTime) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>(), 2.0);
    b.update(1.5);
    EXPECT_FALSE(b.hasExploded());
    b.update(0.5); // timer_ = 2.0 -> >= fuseTime_
    EXPECT_TRUE(b.hasExploded());
}

TEST(Bomb, UpdateAfterExplodingIsNoOp) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>(), 1.0);
    b.explode();
    ASSERT_TRUE(b.hasExploded());
    double timerBefore = b.getTimer();
    b.update(5.0); // mag de timer niet meer laten oplopen (exploded_ guard)
    EXPECT_DOUBLE_EQ(b.getTimer(), timerBefore);
}

TEST(Bomb, ExplodeMarksForRemoval) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>(), 1.0);
    b.explode();
    EXPECT_TRUE(b.isMarkedForRemoval());
}

TEST(Bomb, ExplodeIsIdempotent) {
    // Een tweede explode()-call (bv. door een kettingreactie) mag geen
    // dubbele Removed/BombExploded-notify triggeren; hasExploded() blijft
    // gewoon true en er mag niets crashen.
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>(), 1.0);
    b.explode();
    b.explode();
    EXPECT_TRUE(b.hasExploded());
}

TEST(Bomb, ConsumeDamageFlagReturnsTrueExactlyOnce) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>(), 1.0);
    EXPECT_TRUE(b.consumeDamageFlag());
    EXPECT_FALSE(b.consumeDamageFlag());
    EXPECT_FALSE(b.consumeDamageFlag());
}

TEST(Bomb, RadiusAndFuseTimeGetters) {
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 3, std::weak_ptr<Character>(), 2.5);
    EXPECT_EQ(b.getRadius(), 3);
    EXPECT_DOUBLE_EQ(b.getFuseTime(), 2.5);
}

TEST(Bomb, OwnerIsAccessibleViaWeakPtr) {
    auto owner = std::make_shared<Character>(Vec2(0, 0), Vec2(0.1, 0.1), false);
    Bomb b(Vec2(0, 0), Vec2(0.1, 0.1), 1, owner, 1.0);
    EXPECT_EQ(b.getOwner().lock(), owner);
}

// ---------------- Character ----------------

TEST(Character, DefaultsForPlayer) {
    Character player(Vec2(0, 0), Vec2(0.12, 0.12), /*isBot=*/false);
    EXPECT_TRUE(player.isAlive());
    EXPECT_FALSE(player.isBot());
    EXPECT_EQ(player.getLives(), 3);
    EXPECT_EQ(player.getMaxBombs(), 1);
    EXPECT_EQ(player.getBombRadius(), 1);
    EXPECT_TRUE(player.canPlaceBomb());
}

TEST(Character, BotHasOneLifeAndIsSlower) {
    Character player(Vec2(0, 0), Vec2(0.12, 0.12), false);
    Character bot(Vec2(0, 0), Vec2(0.12, 0.12), true);
    EXPECT_EQ(bot.getLives(), 1);
    EXPECT_LT(bot.getSpeed(), player.getSpeed());
}

TEST(Character, LoseLifeDecrementsUntilDeath) {
    Character player(Vec2(0, 0), Vec2(0.12, 0.12), false);
    player.loseLife();
    EXPECT_EQ(player.getLives(), 2);
    EXPECT_TRUE(player.isAlive());
    EXPECT_TRUE(player.isInvulnerable()); // korte onkwetsbaarheid na treffer
}

TEST(Character, InvulnerabilityBlocksFollowUpHitsUntilItExpires) {
    Character player(Vec2(0, 0), Vec2(0.12, 0.12), false);
    player.loseLife(); // 3 -> 2, invulnerableTimer_ = 1.5s
    player.loseLife(); // genegeerd: nog onkwetsbaar
    EXPECT_EQ(player.getLives(), 2);

    player.update(1.5); // telt de onkwetsbaarheidstimer volledig af
    EXPECT_FALSE(player.isInvulnerable());

    player.loseLife(); // telt nu wel
    EXPECT_EQ(player.getLives(), 1);
}

TEST(Character, DiesAtZeroLives) {
    Character bot(Vec2(0, 0), Vec2(0.12, 0.12), true); // 1 leven
    EXPECT_TRUE(bot.isAlive());
    bot.loseLife();
    EXPECT_EQ(bot.getLives(), 0);
    EXPECT_FALSE(bot.isAlive());
}

TEST(Character, LoseLifeOnDeadCharacterIsNoOp) {
    Character bot(Vec2(0, 0), Vec2(0.12, 0.12), true);
    bot.loseLife(); // dood
    ASSERT_FALSE(bot.isAlive());
    bot.loseLife(); // mag niet onderflowen of crashen
    EXPECT_EQ(bot.getLives(), 0);
}

TEST(Character, DieIsIdempotent) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    c.die();
    EXPECT_FALSE(c.isAlive());
    c.die(); // tweede keer mag niet crashen / geen effect meer hebben
    EXPECT_FALSE(c.isAlive());
}

TEST(Character, BombPlacementRespectsMaxBombsAndCooldown) {
    Character bot(Vec2(0, 0), Vec2(0.12, 0.12), true);
    EXPECT_TRUE(bot.canPlaceBomb());
    bot.onBombPlaced(); // maxBombs_ == 1, dus nu vol + bot-cooldown actief
    EXPECT_FALSE(bot.canPlaceBomb());

    bot.onBombResolved(); // bom is ontploft, slot komt vrij
    EXPECT_FALSE(bot.canPlaceBomb()) << "cooldown moet apart van bombsInPlay_ blijven gelden";

    bot.update(BOT_BOMB_COOLDOWN_SECONDS); // cooldown volledig aftellen
    EXPECT_TRUE(bot.canPlaceBomb());
}

TEST(Character, PlayerHasNoBombCooldownOnlySlotLimit) {
    // Enkel bots krijgen een cooldown (zie onBombPlaced()); de speler wordt
    // enkel door maxBombs_ / bombsInPlay_ tegengehouden.
    Character player(Vec2(0, 0), Vec2(0.12, 0.12), false);
    player.onBombPlaced();
    EXPECT_FALSE(player.canPlaceBomb());
    player.onBombResolved();
    EXPECT_TRUE(player.canPlaceBomb());
}

TEST(Character, DeadCharacterCannotPlaceBomb) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    c.die();
    EXPECT_FALSE(c.canPlaceBomb());
}

TEST(Character, ApplyPowerUpFire) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    c.applyPowerUp(PowerUpType::Fire);
    EXPECT_EQ(c.getBombRadius(), 2);
}

TEST(Character, ApplyPowerUpExtraBomb) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    c.applyPowerUp(PowerUpType::ExtraBomb);
    EXPECT_EQ(c.getMaxBombs(), 2);
}

TEST(Character, ApplyPowerUpSkatesIncreasesSpeed) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    double before = c.getSpeed();
    c.applyPowerUp(PowerUpType::Skates);
    EXPECT_GT(c.getSpeed(), before);
}

TEST(Character, ApplyPowerUpPoisonNeverDropsBelowFloor) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    for (int i = 0; i < 20; ++i)
        c.applyPowerUp(PowerUpType::Poison);
    EXPECT_GE(c.getSpeed(), 0.1);
}

TEST(Character, ApplyPowerUpCurseNeverDropsMaxBombsBelowOne) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    c.applyPowerUp(PowerUpType::Curse); // maxBombs_ was al 1
    EXPECT_EQ(c.getMaxBombs(), 1);
}

TEST(Character, ApplyPowerUpSkullNeverDropsBombRadiusBelowOne) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    c.applyPowerUp(PowerUpType::Skull); // bombRadius_ was al 1
    EXPECT_EQ(c.getBombRadius(), 1);
}

TEST(Character, StandingOnBombTracksWeakPtr) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    auto bomb = std::make_shared<Bomb>(Vec2(0, 0), Vec2(0.1, 0.1), 1, std::weak_ptr<Character>());
    EXPECT_TRUE(c.getStandingOnBomb().expired());
    c.setStandingOnBomb(bomb);
    EXPECT_EQ(c.getStandingOnBomb().lock(), bomb);
}

TEST(Character, DirectionGetterSetter) {
    Character c(Vec2(0, 0), Vec2(0.12, 0.12), false);
    EXPECT_EQ(c.getDirection(), Direction::Down); // default
    c.setDirection(Direction::Left);
    EXPECT_EQ(c.getDirection(), Direction::Left);
}
