#include "Game.h"
#include "Core.h"
#include <string>
#include <stdexcept>
#include <cmath>

using namespace bomberman;

namespace {
    constexpr unsigned WINDOW_SIZE = 700;
    constexpr unsigned HUD_HEIGHT = 56;
}

Game::Game()
    : window_(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE + HUD_HEIGHT), "Bomberman"),
      camera_(WINDOW_SIZE, WINDOW_SIZE) {
    // De enige toegelaten uitzondering op "geen busy waiting": een maximale FPS-cap.
    window_.setFramerateLimit(60);

    // Lettertype laden voor de UI-tekst; als dit faalt tekenen we gewoon geen tekst
    // (voorkomt een crash als het lettertype niet op het systeem staat).
    fontLoaded_ = font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
    if (!spriteSheet_.loadFromFile("assets/spritesheet.png")) {
        throw std::runtime_error("Kon spritesheet.png niet laden");
    }

    startNewGame();
}

void Game::startNewGame() {
    factory_ = std::make_unique<ConcreteFactory>(camera_, spriteSheet_);
    score_ = std::make_shared<Score>();
    world_ = std::make_unique<World>(*factory_, *score_);
    world_->attach(score_); // Observer-patroon: Score luistert naar de events van World
    world_->generateArena();
    stageBannerTimer_ = STAGE_BANNER_DURATION; // toon "STAGE 1" venster bij start
}

void Game::run() {
    while (window_.isOpen()) {
        Stopwatch::instance().tick();
        double dt = Stopwatch::instance().getDeltaTime();

        processInput();
        if (state_ == State::Playing) update(dt);
        render();
    }
}

void Game::processInput() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }

        if (state_ == State::StartScreen && event.type == sf::Event::MouseButtonPressed) {
            // De "Play"-knop beslaat het onderste gedeelte van het venster.
            if (event.mouseButton.y > static_cast<int>(WINDOW_SIZE) / 2) {
                startNewGame();
                state_ = State::Playing;
            }
        }

        if (state_ == State::GameOver && event.type == sf::Event::KeyPressed) {
            state_ = State::StartScreen;
        }
    }

    if (state_ != State::Playing) return;

    // Continue beweging (niet discreet): elke frame wordt de richting doorgegeven,
    // World::update() vermenigvuldigt dit met deltaTime.
    Direction dir = Direction::None;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))         dir = Direction::Up;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dir = Direction::Down;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dir = Direction::Left;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dir = Direction::Right;
    world_->setPlayerDirection(dir);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        world_->requestPlayerBomb();
    }
}

void Game::update(double dt) {
    if (stageBannerTimer_ > 0.0) {
        // Spel staat "on hold" zolang de stage-banner nog getoond wordt.
        stageBannerTimer_ -= dt;
        if (stageBannerTimer_ < 0.0) stageBannerTimer_ = 0.0;
        return;
    }
    world_->update(dt);
    factory_->removeExpiredViews();

    // Speler heeft de verborgen deur bereikt: World is al naar de volgende
    // stage gegaan (nieuw doolhof, zelfde speler/score). Toon opnieuw de
    // korte "STAGE X"-banner, net als bij de allereerste start.
    if (world_->consumeStageAdvanced()) {
        stageBannerTimer_ = STAGE_BANNER_DURATION;
    }
    if (world_->isGameOver()) state_ = State::GameOver;
}

void Game::render() {
    window_.clear(sf::Color(20, 20, 24));

    if (state_ == State::StartScreen) {
        // Startscherm blijft over het volledige venster (default view).
        window_.setView(window_.getDefaultView());

        // ---- Grote "BOMBERMAN"-titel in pixel/arcade-stijl: dikke donkerrode
        // "schaduw"-laag eronder + oranje/gele hoofdtekst erboven, net als op
        // de originele boxart. Geen anti-aliasing -> hardere, pixelige randen. ----
        sf::Text titleShadow("BOMBERMAN", font_, 56);
        titleShadow.setStyle(sf::Text::Bold);
        titleShadow.setFillColor(sf::Color(140, 20, 20));
        titleShadow.setPosition(WINDOW_SIZE / 2.f - 192.f + 5.f, 36.f + 5.f);
        if (fontLoaded_) window_.draw(titleShadow);

        sf::Text title("BOMBERMAN", font_, 56);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color(255, 165, 0));   // oranje
        title.setOutlineColor(sf::Color(150, 30, 10)); // donkerrode/bruine rand
        title.setOutlineThickness(3.f);
        title.setPosition(WINDOW_SIZE / 2.f - 192.f, 36.f);
        if (fontLoaded_) window_.draw(title);

        // ---- Top 5 scores: gecentreerde box, lichtgrijs t.o.v. de donkere achtergrond ----
        {
            const auto& highScores = score_->getHighScores();
            constexpr float BOX_WIDTH = 220.f;
            constexpr float TITLE_H = 44.f;
            constexpr float ROW_H = 28.f;
            float boxHeight = TITLE_H + ROW_H * static_cast<float>(highScores.size()) + 14.f;
            float boxX = WINDOW_SIZE / 2.f - BOX_WIDTH / 2.f;
            float boxY = 150.f;

            sf::RectangleShape scoreBox(sf::Vector2f(BOX_WIDTH, boxHeight));
            scoreBox.setPosition(boxX, boxY);
            scoreBox.setFillColor(sf::Color(48, 48, 54));       // lichter grijs dan de achtergrond
            scoreBox.setOutlineColor(sf::Color(90, 92, 100));
            scoreBox.setOutlineThickness(2.f);
            window_.draw(scoreBox);

            sf::Text scoresLabel("Top 5 scores", font_, 20);
            scoresLabel.setStyle(sf::Text::Bold);
            scoresLabel.setFillColor(sf::Color(255, 200, 120));
            if (fontLoaded_) {
                sf::FloatRect lb = scoresLabel.getLocalBounds();
                scoresLabel.setPosition(WINDOW_SIZE / 2.f - lb.width / 2.f - lb.left, boxY + 12.f);
                window_.draw(scoresLabel);
            }

            float y = boxY + TITLE_H;
            for (int s : highScores) {
                sf::Text line(std::to_string(s), font_, 18);
                line.setFillColor(sf::Color::White);
                if (fontLoaded_) {
                    sf::FloatRect lbnds = line.getLocalBounds();
                    line.setPosition(WINDOW_SIZE / 2.f - lbnds.width / 2.f - lbnds.left, y);
                    window_.draw(line);
                }
                y += ROW_H;
            }
        }

        // Speel-knop: oranje i.p.v. groen, met opschrift "BOMBERMAAN".
        sf::RectangleShape button(sf::Vector2f(240, 60));
        button.setPosition(WINDOW_SIZE / 2.f - 120, WINDOW_SIZE - 150.f);
        button.setFillColor(sf::Color(235, 140, 20));
        button.setOutlineColor(sf::Color(140, 20, 20));
        button.setOutlineThickness(3.f);
        window_.draw(button);

        sf::Text playText("PLAY", font_, 24);
        playText.setStyle(sf::Text::Bold);
        playText.setFillColor(sf::Color(60, 20, 10));
        if (fontLoaded_) {
            sf::FloatRect pb = playText.getLocalBounds();
            playText.setPosition(
                WINDOW_SIZE / 2.f - pb.width / 2.f - pb.left,
                WINDOW_SIZE - 150.f + 30.f - pb.height / 2.f - pb.top
            );
            window_.draw(playText);
        }
    } else {
        // ---- Spelveld: verschoven onder de HUD-balk via een aparte view ----
        sf::View gameView(sf::FloatRect(0.f, 0.f, static_cast<float>(WINDOW_SIZE), static_cast<float>(WINDOW_SIZE)));
        float totalH = static_cast<float>(WINDOW_SIZE + HUD_HEIGHT);
        gameView.setViewport(sf::FloatRect(0.f, HUD_HEIGHT / totalH, 1.f, WINDOW_SIZE / totalH));
        window_.setView(gameView);

        // Vloer-tegels als achtergrond, licht dambordpatroon.
        // Effen donkergroene vloer/gras-look, geen dambordpatroon.
        {
            int cols = bomberman::GRID_COLS, rows = bomberman::GRID_ROWS;
            float tileW = static_cast<float>(WINDOW_SIZE) / cols;
            float tileH = static_cast<float>(WINDOW_SIZE) / rows;
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    sf::RectangleShape tile(sf::Vector2f(tileW, tileH));
                    tile.setPosition(c * tileW, r * tileH);
                    tile.setFillColor(sf::Color(58, 120, 56)); // effen donkergroen
                    window_.draw(tile);
                }
            }
        }

        factory_->drawAll(window_);

        // ---- HUD-balk: terug naar de volledige-venster view ----
        window_.setView(window_.getDefaultView());

        sf::RectangleShape hudBar(sf::Vector2f(static_cast<float>(WINDOW_SIZE), static_cast<float>(HUD_HEIGHT)));
        hudBar.setPosition(0.f, 0.f);
        hudBar.setFillColor(sf::Color(24, 26, 32));
        window_.draw(hudBar);

        auto player = world_->getPlayer();

        // ---- HUD-items: Score, Bommen, Bereik, Leven, Tijd. Elke volgende positie wordt
        // berekend op basis van de echte breedte van het vorige item + vaste marge,
        // zodat items nooit overlappen ongeacht de lengte van de tekst. ----
        {
            constexpr float HUD_MARGIN = 10.f;
            constexpr float HUD_GAP = 30.f; // ruimte tussen items

            float x = HUD_MARGIN;

            sf::Text scoreText("Score: " + std::to_string(score_->getCurrentScore()), font_, 18);
            scoreText.setPosition(x, 17.f);
            scoreText.setFillColor(sf::Color(255, 215, 0)); // goud
            if (fontLoaded_) window_.draw(scoreText);
            x += scoreText.getLocalBounds().width + HUD_GAP;

            if (player) {
                std::string bombStr = "Bommen: " + std::to_string(player->getBombsInPlay())
                                     + "/" + std::to_string(player->getMaxBombs());
                sf::Text bombText(bombStr, font_, 18);
                bombText.setPosition(x, 17.f);
                bombText.setFillColor(sf::Color(255, 90, 90)); // rood
                if (fontLoaded_) window_.draw(bombText);
                x += bombText.getLocalBounds().width + HUD_GAP;

                sf::Text radiusText("Bereik: " + std::to_string(player->getBombRadius()), font_, 18);
                radiusText.setPosition(x, 17.f);
                radiusText.setFillColor(sf::Color(90, 200, 255)); // blauw
                if (fontLoaded_) window_.draw(radiusText);
                x += radiusText.getLocalBounds().width + HUD_GAP;

                sf::Text livesText("Leven: " + std::to_string(player->getLives()), font_, 18);
                livesText.setPosition(x, 17.f);
                livesText.setFillColor(sf::Color(255, 90, 200)); // roze
                if (fontLoaded_) window_.draw(livesText);
                x += livesText.getLocalBounds().width + HUD_GAP;
            }

            // Timer: mm:ss, resterende tijd voor dit level.
            int totalSeconds = static_cast<int>(std::ceil(world_->getTimeRemaining()));
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;
            std::string secStr = (seconds < 10 ? "0" : "") + std::to_string(seconds);
            sf::Text timerText("Tijd: " + std::to_string(minutes) + ":" + secStr, font_, 18);
            timerText.setPosition(x, 17.f);
            timerText.setFillColor(totalSeconds <= 30 ? sf::Color(255, 60, 60) : sf::Color(200, 200, 200));
            if (fontLoaded_) window_.draw(timerText);
        }

        // ---- "STAGE 1"-venster: kort getoond bovenop het speelveld bij de start. ----
        if (stageBannerTimer_ > 0.0) {
            sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(WINDOW_SIZE), static_cast<float>(WINDOW_SIZE + HUD_HEIGHT)));
            overlay.setPosition(0.f, 0.f);
            overlay.setFillColor(sf::Color(0, 0, 0, 160));
            window_.draw(overlay);

            sf::Text stageText("STAGE " + std::to_string(world_->getCurrentLevel()), font_, 48);
            stageText.setStyle(sf::Text::Bold);
            stageText.setFillColor(sf::Color(255, 165, 0));
            stageText.setOutlineColor(sf::Color(150, 30, 10));
            stageText.setOutlineThickness(3.f);
            stageText.setPosition(WINDOW_SIZE / 2.f - 90.f, (WINDOW_SIZE + HUD_HEIGHT) / 2.f - 30.f);
            if (fontLoaded_) window_.draw(stageText);
        }

        if (state_ == State::GameOver) {
            sf::Text msg(world_->didPlayerWin() ? "YOU WIN! Press any key" : "GAME OVER! Press any key",
                         font_, 26);
            msg.setPosition(WINDOW_SIZE / 2.f - 220, WINDOW_SIZE / 2.f + HUD_HEIGHT);
            msg.setFillColor(sf::Color::Yellow);
            if (fontLoaded_) window_.draw(msg);
        }
    }

    window_.display();
}