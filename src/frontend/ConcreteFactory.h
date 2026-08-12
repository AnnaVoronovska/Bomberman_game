#pragma once
#include "AbstractFactory.h"
#include "AudioManager.h"
#include "Views.h"
#include <memory>
#include <vector>

// ============================================================
// ConcreteFactory.hpp
// Concrete Factory: maakt logic-entities aan EN koppelt er meteen (via
// het Observer-patroon) de juiste SFML-View én de AudioManager aan. World
// gebruikt deze klasse enkel via de AbstractFactory-interface en weet dus
// niet dat Views of geluid bestaan.
// ============================================================

class ConcreteFactory : public bomberman::AbstractFactory {
public:
    ConcreteFactory(const bomberman::Camera& camera, const sf::Texture& texture,
                    std::shared_ptr<AudioManager> audio = nullptr);

    std::shared_ptr<bomberman::Character> createCharacter(bomberman::Vec2 position, bool isBot) override;
    std::shared_ptr<bomberman::Wall> createWall(bomberman::Vec2 position, bool destructible) override;
    std::shared_ptr<bomberman::Bomb> createBomb(bomberman::Vec2 position, int radius,
                                                std::weak_ptr<bomberman::Character> owner) override;
    std::shared_ptr<bomberman::PowerUp> createPowerUp(bomberman::Vec2 position, bomberman::PowerUpType type) override;
    std::shared_ptr<bomberman::Door> createDoor(bomberman::Vec2 position) override;

    void drawAll(sf::RenderWindow& window);
    void removeExpiredViews();

private:
    const bomberman::Camera& camera_;
    const sf::Texture& texture_;
    std::shared_ptr<AudioManager> audio_; // optioneel: nullptr = geen geluid (bv. in tests)
    std::vector<std::shared_ptr<EntityView>> views_; // representatie-laag bezit de Views
    int botIndex_ = 0;
};