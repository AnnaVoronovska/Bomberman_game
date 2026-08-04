#include "Game.h"
#include "Core.h"
#include <string>
#include <stdexcept>

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
    world_->update(dt);
    factory_->removeExpiredViews();
    if (world_->isGameOver()) state_ = State::GameOver;
}

void Game::render() {
    window_.clear(sf::Color(20, 20, 24));

    if (state_ == State::StartScreen) {
        // Startscherm blijft over het volledige venster (default view).
        window_.setView(window_.getDefaultView());

        sf::Text title("BOMBERMAN", font_, 40);
        title.setPosition(WINDOW_SIZE / 2.f - 140, 40);
        if (fontLoaded_) window_.draw(title);

        sf::Text scoresLabel("Top 5 scores:", font_, 22);
        scoresLabel.setPosition(60, 140);
        if (fontLoaded_) window_.draw(scoresLabel);

        float y = 180.f;
        for (int s : score_->getHighScores()) {
            sf::Text line(std::to_string(s), font_, 20);
            line.setPosition(80, y);
            if (fontLoaded_) window_.draw(line);
            y += 30.f;
        }

        sf::RectangleShape button(sf::Vector2f(200, 60));
        button.setPosition(WINDOW_SIZE / 2.f - 100, WINDOW_SIZE - 150.f);
        button.setFillColor(sf::Color(60, 160, 60));
        window_.draw(button);

        sf::Text playText("PLAY", font_, 28);
        playText.setPosition(WINDOW_SIZE / 2.f - 35, WINDOW_SIZE - 138.f);
        if (fontLoaded_) window_.draw(playText);
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

        sf::Text scoreText("Score: " + std::to_string(score_->getCurrentScore()), font_, 20);
        scoreText.setPosition(12.f, 16.f);
        scoreText.setFillColor(sf::Color(255, 215, 0)); // goud
        if (fontLoaded_) window_.draw(scoreText);

        if (player) {
            std::string bombStr = "Bommen: " + std::to_string(player->getBombsInPlay())
                                 + "/" + std::to_string(player->getMaxBombs());
            sf::Text bombText(bombStr, font_, 20);
            bombText.setPosition(220.f, 16.f);
            bombText.setFillColor(sf::Color(255, 90, 90)); // rood
            if (fontLoaded_) window_.draw(bombText);

            sf::Text radiusText("Bereik: " + std::to_string(player->getBombRadius()), font_, 20);
            radiusText.setPosition(410.f, 16.f);
            radiusText.setFillColor(sf::Color(90, 200, 255)); // blauw
            if (fontLoaded_) window_.draw(radiusText);
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
