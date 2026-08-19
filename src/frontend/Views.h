#pragma once
#include "Core.h"
#include "Entities.h"
#include "Observer.h"
#include <SFML/Graphics.hpp>
#include <memory>

// ============================================================
// Views.h
// View-deel van MVC. Elke EntityView abonneert zich (Observer) op zijn
// bijhorend Model, en weet hoe hij zichzelf met SFML moet tekenen.
// Simpele gekleurde vormen worden gebruikt als "sprite" - zie README.md
// voor hoe je dit vervangt door een echte sprite sheet.
// ============================================================

/**
 * @brief Basisklasse (View) voor de grafische weergave van een EntityModel.
 * Abonneert zich via het Observer-patroon op zijn bijhorend Model.
 */
class EntityView : public bomberman::Observer {
public:
    EntityView(std::weak_ptr<bomberman::EntityModel> model, const bomberman::Camera& camera);
    virtual ~EntityView() = default;

    void onNotify(const bomberman::Event& event) override;
    /// @brief Tekent deze entiteit op het venster; deltaTime dient voor animatietiming.
    virtual void draw(sf::RenderWindow& window, double deltaTime) = 0;
    /// @brief Geeft terug of het onderliggende Model verdwenen is en de View opgeruimd mag worden.
    virtual bool isExpired() const;

protected:
    std::weak_ptr<bomberman::EntityModel> model_;
    const bomberman::Camera& camera_;
    bool removed_ = false;
};

/// @brief View voor een Wall: tekent een breekbare of onverwoestbare muurtegel.
class WallView : public EntityView {
public:
    WallView(std::weak_ptr<bomberman::Wall> model, const bomberman::Camera& camera, const sf::Texture& texture);
    void draw(sf::RenderWindow& window, double deltaTime) override;

private:
    std::weak_ptr<bomberman::Wall> wall_;
    const sf::Texture* texture_ = nullptr;
};

/// @brief View voor een PowerUp: tekent het sprite dat bij het power-up type hoort.
class PowerUpView : public EntityView {
public:
    PowerUpView(std::weak_ptr<bomberman::PowerUp> model, const bomberman::Camera& camera, const sf::Texture& texture);
    void draw(sf::RenderWindow& window, double deltaTime) override;

private:
    std::weak_ptr<bomberman::PowerUp> powerUp_;
    const sf::Texture* texture_ = nullptr;
};

/// @brief View voor een Door: tekent de verborgen deur met een pulserend effect.
class DoorView : public EntityView {
public:
    DoorView(std::weak_ptr<bomberman::Door> model, const bomberman::Camera& camera);
    void draw(sf::RenderWindow& window, double deltaTime) override;

private:
    std::weak_ptr<bomberman::Door> door_;
    double pulse_ = 0.0;
};

/// @brief View voor een Bomb: tekent het aftellende bom-sprite en de explosie-animatie.
class BombView : public EntityView {
public:
    BombView(std::weak_ptr<bomberman::Bomb> model, const bomberman::Camera& camera, const sf::Texture& texture);
    void onNotify(const bomberman::Event& event) override;
    void draw(sf::RenderWindow& window, double deltaTime) override;
    bool isExpired() const override;

private:
    std::weak_ptr<bomberman::Bomb> bomb_;
    const sf::Texture* texture_ = nullptr;
    double pulse_ = 0.0;

    bool exploding_ = false;
    double explodeAnimTimer_ = 0.0;
    bomberman::Vec2 lastPos_;
    bomberman::Vec2 lastSize_;
};

/**
 * @brief View voor een Character (Player of bot): tekent de richtingsgebonden
 * loop-animatie en de death-/victory-animaties.
 */
class CharacterView : public EntityView {
public:
    // rowBase/colBase = linkerbovenhoek (in tegels) van het "down"-frame van deze skin
    // in de spritesheet. up = rowBase+1, left = rowBase+2 (zelfde colBase).
    CharacterView(std::weak_ptr<bomberman::Character> model, const bomberman::Camera& camera,
                  const sf::Texture& texture, int rowBase, int colBase = 0);
    void onNotify(const bomberman::Event& event) override;
    void draw(sf::RenderWindow& window, double deltaTime) override;

private:
    std::weak_ptr<bomberman::Character> character_;
    const sf::Texture* texture_ = nullptr;
    int rowBase_ = 1;
    int colBase_ = 0;

    // Loop-animatie state.
    double animTimer_ = 0.0;
    int frame_ = 0;
    bomberman::Direction lastDirection_ = bomberman::Direction::Down;

    // Death-animatie state: gestart zodra het Model een Died-event stuurt.
    bool dying_ = false;
    double deathAnimTimer_ = 0.0;

    // Victory-animatie state: gestart zodra de Player een PlayerWon-event
    // ontvangt (enkel relevant voor de View van de Player).
    bool won_ = false;
    double winAnimTimer_ = 0.0;
};