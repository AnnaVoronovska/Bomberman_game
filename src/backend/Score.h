#pragma once
#include "Observer.h"
#include <string>
#include <vector>

// ============================================================
// Score.h
// Observer die luistert naar spel-events (via World::notify) en er
// score-aanpassingen van maakt. Beheert ook de top-5 hoogste scores,
// die naar een tekstbestand geschreven worden zodat ze bewaard
// blijven tussen verschillende keren dat je het spel speelt.
// ============================================================

namespace bomberman {

    class Score : public Observer {
    public:
        Score();

        void onNotify(const Event& event) override;

        int getCurrentScore() const { return currentScore_; }
        void reset();

        void loadHighScores(const std::string& path);
        void saveHighScores(const std::string& path) const;
        const std::vector<int>& getHighScores() const { return highScores_; }
        void registerFinalScore(); // stopt currentScore_ in de top-5 tabel

    private:
        int currentScore_ = 0;
        std::vector<int> highScores_;
        std::string highScorePath_ = "highscores.txt";
    };

} // namespace bomberman