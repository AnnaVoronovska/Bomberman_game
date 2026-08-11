#include "Score.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>

using namespace bomberman;

// Score gebruikt intern een vast pad ("highscores.txt") voor
// registerFinalScore/de constructor. We ruimen dat bestand netjes op
// voor en na elke test, zodat tests elkaar niet beïnvloeden en er geen
// rommel achterblijft in de build-map.
class ScoreTest : public ::testing::Test {
protected:
    void SetUp() override { std::remove("highscores.txt"); }
    void TearDown() override { std::remove("highscores.txt"); }
};

TEST_F(ScoreTest, StartsAtZero) {
    Score score;
    EXPECT_EQ(score.getCurrentScore(), 0);
}

TEST_F(ScoreTest, TickAddsProportionalBonus) {
    Score score;
    score.onNotify(Event{EventType::Tick, nullptr, 0.1}); // 0.1 * 5 = 0.5 -> (int) 0
    score.onNotify(Event{EventType::Tick, nullptr, 1.0}); // 1.0 * 5 = 5
    EXPECT_EQ(score.getCurrentScore(), 5);
}

TEST_F(ScoreTest, BlockDestroyedAddsTenPoints) {
    Score score;
    score.onNotify(Event{EventType::BlockDestroyed, nullptr});
    EXPECT_EQ(score.getCurrentScore(), 10);
}

TEST_F(ScoreTest, PowerUpCollectedAddsTwentyFivePoints) {
    Score score;
    score.onNotify(Event{EventType::PowerUpCollected, nullptr});
    EXPECT_EQ(score.getCurrentScore(), 25);
}

TEST_F(ScoreTest, EnemyKilledAddsOneHundredFiftyPoints) {
    Score score;
    score.onNotify(Event{EventType::EnemyKilled, nullptr});
    EXPECT_EQ(score.getCurrentScore(), 150);
}

TEST_F(ScoreTest, PlayerWonAddsFiveHundredPoints) {
    Score score;
    score.onNotify(Event{EventType::BlockDestroyed, nullptr}); // 10
    score.onNotify(Event{EventType::PlayerWon, nullptr});      // +500
    EXPECT_EQ(score.getCurrentScore(), 510);
}

TEST_F(ScoreTest, PlayerDiedSubtractsFiftyPointsButNeverGoesNegative) {
    Score score;
    score.onNotify(Event{EventType::BlockDestroyed, nullptr}); // 10
    score.onNotify(Event{EventType::PlayerDied, nullptr});     // max(0, 10-50) = 0
    EXPECT_EQ(score.getCurrentScore(), 0);
}

TEST_F(ScoreTest, PlayerDiedNeverGoesBelowZeroFromHighScore) {
    Score score;
    for (int i = 0; i < 3; ++i)
        score.onNotify(Event{EventType::BlockDestroyed, nullptr}); // 30
    score.onNotify(Event{EventType::PlayerDied, nullptr});         // 30-50 -> clamp naar 0
    EXPECT_EQ(score.getCurrentScore(), 0);
}

TEST_F(ScoreTest, UnhandledEventTypesAreIgnored) {
    Score score;
    score.onNotify(Event{EventType::Moved, nullptr});
    score.onNotify(Event{EventType::Damaged, nullptr});
    score.onNotify(Event{EventType::Removed, nullptr});
    EXPECT_EQ(score.getCurrentScore(), 0);
}

TEST_F(ScoreTest, ResetSetsScoreBackToZero) {
    Score score;
    score.onNotify(Event{EventType::EnemyKilled, nullptr});
    ASSERT_GT(score.getCurrentScore(), 0);
    score.reset();
    EXPECT_EQ(score.getCurrentScore(), 0);
}

TEST_F(ScoreTest, SaveAndLoadHighScoresRoundTrip) {
    Score score;
    score.saveHighScores("test_highscores.tmp");

    // Simuleer een handmatig bestand met scores die niet gesorteerd zijn.
    {
        std::ofstream f("test_highscores.tmp");
        f << "100\n300\n200\n50\n400\n999\n"; // 6 waarden, top-5 verwacht
    }
    score.loadHighScores("test_highscores.tmp");

    const auto& scores = score.getHighScores();
    ASSERT_EQ(scores.size(), 5u); // afgekapt tot top 5
    EXPECT_EQ(scores[0], 999);
    EXPECT_EQ(scores[1], 400);
    EXPECT_EQ(scores[2], 300);
    EXPECT_EQ(scores[3], 200);
    EXPECT_EQ(scores[4], 100); // 50 valt eraf

    std::remove("test_highscores.tmp");
}

TEST_F(ScoreTest, LoadHighScoresFromMissingFileYieldsEmptyList) {
    Score score;
    score.loadHighScores("this_file_does_not_exist.tmp");
    EXPECT_TRUE(score.getHighScores().empty());
}

TEST_F(ScoreTest, RegisterFinalScoreInsertsCurrentScoreIntoHighScores) {
    Score score;
    score.loadHighScores("nonexistent_seed.tmp"); // begin met een lege lijst
    score.onNotify(Event{EventType::EnemyKilled, nullptr}); // currentScore_ = 150
    score.registerFinalScore();

    const auto& scores = score.getHighScores();
    ASSERT_FALSE(scores.empty());
    EXPECT_EQ(scores[0], 150);
}

TEST_F(ScoreTest, PlayerDiedAutomaticallyRegistersFinalScore) {
    Score score;
    score.loadHighScores("nonexistent_seed2.tmp");
    score.onNotify(Event{EventType::BlockDestroyed, nullptr}); // 10
    score.onNotify(Event{EventType::PlayerDied, nullptr});     // clamp 0, en registreert 0
    EXPECT_FALSE(score.getHighScores().empty());
    EXPECT_EQ(score.getHighScores()[0], 0);
}
