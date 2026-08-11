#include "Score.h"
#include <algorithm>
#include <fstream>

namespace bomberman {

Score::Score() { loadHighScores(highScorePath_); }

void Score::onNotify(const Event& event) {
    switch (event.type) {
    case EventType::Tick:
        // kleine, continue bonus voor hoe lang de Player al leeft
        currentScore_ += static_cast<int>(event.value * 5.0);
        break;
    case EventType::BlockDestroyed:
        currentScore_ += 10;
        break;
    case EventType::PowerUpCollected:
        currentScore_ += 25;
        break;
    case EventType::EnemyKilled:
        currentScore_ += 150;
        break;
    case EventType::PlayerDied:
        currentScore_ = std::max(0, currentScore_ - 50); // penalty voor verliezen
        registerFinalScore();
        break;
    case EventType::PlayerWon:
        currentScore_ += 500; // bonus voor winnen
        registerFinalScore();
        break;
    default:
        break;
    }
}

void Score::reset() { currentScore_ = 0; }

void Score::loadHighScores(const std::string& path) {
    highScores_.clear();
    std::ifstream file(path);
    int value;
    while (file >> value)
        highScores_.push_back(value);
    std::sort(highScores_.rbegin(), highScores_.rend());
    if (highScores_.size() > 5)
        highScores_.resize(5);
}

void Score::saveHighScores(const std::string& path) const {
    std::ofstream file(path);
    for (int value : highScores_)
        file << value << "\n";
}

void Score::registerFinalScore() {
    highScores_.push_back(currentScore_);
    std::sort(highScores_.rbegin(), highScores_.rend());
    if (highScores_.size() > 5)
        highScores_.resize(5);
    saveHighScores(highScorePath_);
}

} // namespace bomberman
