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

    /**
     * @brief Observer die luistert naar spel-events en er score-aanpassingen van
     * maakt. Beheert ook de top-5 hoogste scores, opgeslagen in een tekstbestand.
     */
    class Score : public Observer {
    public:
        Score();

        /// @brief Verwerkt een binnenkomend Event tot een aanpassing van de huidige score.
        void onNotify(const Event& event) override;

        int getCurrentScore() const { return currentScore_; }
        /// @brief Zet de huidige score terug naar 0 (bv. bij een nieuwe game).
        void reset();

        /// @brief Laadt de top-5 scores uit het opgegeven bestand.
        void loadHighScores(const std::string& path);
        /// @brief Slaat de top-5 scores op naar het opgegeven bestand.
        void saveHighScores(const std::string& path) const;
        const std::vector<int>& getHighScores() const { return highScores_; }
        /// @brief Voegt de huidige score toe aan de top-5 tabel, indien hoog genoeg.
        void registerFinalScore(); // stopt currentScore_ in de top-5 tabel

    private:
        int currentScore_ = 0;
        std::vector<int> highScores_;
        std::string highScorePath_ = "highscores.txt";
    };

} // namespace bomberman