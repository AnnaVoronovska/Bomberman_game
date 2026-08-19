#pragma once
#include "Observer.h"
#include <SFML/Audio.hpp>
#include <array>
#include <string>
#include <vector>

// ============================================================
// AudioManager.hpp
// Speelt geluidseffecten en achtergrondmuziek af als reactie op Events uit
// de logic-library - exact het Observer-patroon dat Views al gebruiken om
// te tekenen, nu voor geluid. AudioManager wordt op twee plaatsen
// aangesloten:
//   1) op World zelf (world_->attach), voor events die World rechtstreeks
//      notify't: BombPlaced, PlayerDied, PlayerWon, EnemyKilled.
//   2) op elke entiteit via ConcreteFactory::create*() (model->attach),
//      voor events die de entiteit zelf notify't: BombExploded,
//      BlockDestroyed, PowerUpCollected, Damaged, Died.
// Die twee groepen events overlappen niet, dus er is geen risico op een
// geluid dat dubbel afspeelt voor dezelfde gebeurtenis.
//
// Ontbrekende geluidsbestanden zijn geen fatale fout: net als het lettertype
// in Game.cpp wordt een missend bestand gewoon overgeslagen (een waarschuwing
// op stderr) zodat het spel ook zonder audio-assets blijft draaien.
// ============================================================

/**
 * @brief Speelt geluidseffecten en achtergrondmuziek af als reactie op Events
 * uit de logic-library (zie uitleg bovenaan dit bestand voor de aansluitpunten).
 */
class AudioManager : public bomberman::Observer {
public:
    AudioManager();

    /// @brief Speelt het gepaste geluidseffect af op basis van het binnenkomende Event.
    void onNotify(const bomberman::Event& event) override;

    /**
     * @brief Wordt door Game aangeroepen op het moment dat de speler de deur bereikt
     * en naar een volgend level gaat (geen apart Event-type nodig hiervoor:
     * Game::update() detecteert dit al via World::consumeStageAdvanced()).
     */
    void playNextLevelSound();

    /**
     * @brief Wordt door Game aangeroepen zodra de "Play"-knop op het startscherm
     * ingedrukt wordt (geen World/Event-koppeling nodig: puur UI-feedback).
     */
    void playClickSound();

    /// @brief Start achtergrondmuziek vanaf het opgegeven pad, optioneel in lus.
    void playMusic(const std::string& path, bool loop = true);
    /// @brief Pauzeert de huidige achtergrondmuziek.
    void pauseMusic();
    /// @brief Stopt de achtergrondmuziek volledig.
    void stopMusic();
    /// @brief Zet het muziekvolume (0-100).
    void setMusicVolume(float volume); // 0-100
    /// @brief Zet het volume van geluidseffecten (0-100).
    void setSfxVolume(float volume);   // 0-100

private:
    enum class Sfx { BombExplode, PowerUp, Death, Win, NextLevel, PlayClick, Count };
    static constexpr std::size_t SFX_COUNT = static_cast<std::size_t>(Sfx::Count);

    void loadSound(Sfx id, const std::string& path);
    void play(Sfx id);

    std::array<sf::SoundBuffer, SFX_COUNT> buffers_;
    std::array<bool, SFX_COUNT> loaded_{};

    // Pool van sf::Sound-objecten: één sf::Sound kan maar 1 buffer tegelijk
    // afspelen, maar bij een kettingreactie ontploffen meerdere bommen na
    // elkaar in dezelfde/aangrenzende frames. Een kleine ronde-robin pool
    // laat die geluiden overlappen i.p.v. elkaar af te kappen.
    static constexpr std::size_t POOL_SIZE = 16;
    std::vector<sf::Sound> pool_;
    std::size_t nextSlot_ = 0;

    sf::Music music_;
    std::string currentMusicPath_; // om te weten of playMusic() moet hervatten i.p.v. herstarten
    float sfxVolume_ = 70.f;
};