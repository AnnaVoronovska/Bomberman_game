#include "AudioManager.h"
#include <iostream>

using namespace bomberman;

AudioManager::AudioManager() {
    pool_.resize(POOL_SIZE);

    // Bewust minimale set: enkel de geluiden die echt iets toevoegen. Te veel
    // sfx (bomb_place, block_break, hit, enemy_killed) overlapt elkaar
    // tijdens een explosie en wordt snel storend i.p.v. leuk.
    loadSound(Sfx::BombExplode, "assets/sounds/bomb_explode.wav");
    loadSound(Sfx::PowerUp, "assets/sounds/powerup.wav");
    loadSound(Sfx::Death, "assets/sounds/death.wav");
    loadSound(Sfx::Win, "assets/sounds/win.wav");
    loadSound(Sfx::NextLevel, "assets/sounds/next_level.wav");
    loadSound(Sfx::PlayClick, "assets/sounds/play.wav");
}

void AudioManager::loadSound(Sfx id, const std::string& path) {
    std::size_t idx = static_cast<std::size_t>(id);
    if (buffers_[idx].loadFromFile(path)) {
        loaded_[idx] = true;
    } else {
        std::cerr << "[AudioManager] kon geluid niet laden: " << path << " (overgeslagen)\n";
        loaded_[idx] = false;
    }
}

void AudioManager::play(Sfx id) {
    std::size_t idx = static_cast<std::size_t>(id);
    if (!loaded_[idx])
        return;

    sf::Sound& slot = pool_[nextSlot_];
    nextSlot_ = (nextSlot_ + 1) % POOL_SIZE;
    slot.setBuffer(buffers_[idx]);
    slot.setVolume(sfxVolume_);
    slot.play();
}

void AudioManager::onNotify(const Event& event) {
    switch (event.type) {
    case EventType::BombExploded:
        play(Sfx::BombExplode);
        break;
    case EventType::PowerUpCollected:
        play(Sfx::PowerUp);
        break;
    case EventType::Died:
        play(Sfx::Death);
        break;
    case EventType::PlayerWon:
        play(Sfx::Win);
        break;
    default:
        // BombPlaced, BlockDestroyed, Damaged, EnemyKilled, PlayerDied, Tick,
        // Moved, Removed: bewust genegeerd (zie constructor-comment hierboven).
        break;
    }
}

void AudioManager::playNextLevelSound() { play(Sfx::NextLevel); }

void AudioManager::playClickSound() { play(Sfx::PlayClick); }

void AudioManager::playMusic(const std::string& path, bool loop) {
    // Zelfde track opnieuw aangevraagd (bv. "Play" ingedrukt na Game Over):
    // NIET opnieuw vanaf 0 starten, gewoon hervatten waar die gestopt was.
    if (currentMusicPath_ == path) {
        if (music_.getStatus() != sf::Music::Playing)
            music_.play();
        return;
    }

    if (!music_.openFromFile(path)) {
        std::cerr << "[AudioManager] kon muziek niet laden: " << path << " (overgeslagen)\n";
        currentMusicPath_.clear();
        return;
    }
    currentMusicPath_ = path;
    music_.setLoop(loop);
    music_.play();
}

void AudioManager::pauseMusic() { music_.pause(); } // positie blijft behouden, in tegenstelling tot stop()

void AudioManager::stopMusic() { music_.stop(); }

void AudioManager::setMusicVolume(float volume) { music_.setVolume(volume); }

void AudioManager::setSfxVolume(float volume) { sfxVolume_ = volume; }