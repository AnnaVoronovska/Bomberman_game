#pragma once
#include <vector>
#include <memory>

// ============================================================
// Observer.hpp
// Implementatie van het Observer design pattern. EntityModel (elk
// spel-object) en World erven van Subject, en sturen Events naar al hun
// Observers (bv. Views in de representatie-laag, of de Score-klasse).
// ============================================================

namespace bomberman {

class EntityModel; // forward declaration, enkel gebruikt als (niet-eigenaar) pointer

enum class EventType {
    Tick,             // één update-stap is voorbij (voor Score: tijd-bonus)
    Moved,            // entity is van positie/richting veranderd
    Died,             // een character is gestorven (voor animaties)
    BlockDestroyed,   // een breekbare muur werd vernield
    PowerUpCollected, // een power-up werd opgeraapt door de Player
    BombPlaced,       // een bom werd geplaatst
    BombExploded,     // een bom is ontploft
    Removed,          // entity moet verwijderd worden uit World/Views
    EnemyKilled,      // een bot werd gedood door de Player
    PlayerDied,       // de Player is gestorven
    PlayerWon         // de Player heeft gewonnen
};

// Lichtgewicht event dat van een Subject naar zijn Observers gestuurd wordt.
struct Event {
    EventType type;
    EntityModel* source = nullptr; // niet-eigenaar, enkel geldig tijdens de notify-call
    double value = 0.0;            // extra data, bv. deltaTime bij Tick
};

// Observer-kant van het patroon.
class Observer {
public:
    virtual ~Observer() = default;
    virtual void onNotify(const Event& event) = 0;
};

// Subject-kant van het patroon.
class Subject {
public:
    virtual ~Subject() = default;

    void attach(const std::shared_ptr<Observer>& observer) {
        observers_.push_back(observer);
    }

    void notify(const Event& event) {
        for (auto it = observers_.begin(); it != observers_.end(); ) {
            if (auto obs = it->lock()) {
                obs->onNotify(event);
                ++it;
            } else {
                it = observers_.erase(it); // observer bestaat niet meer, opruimen
            }
        }
    }

private:
    std::vector<std::weak_ptr<Observer>> observers_;
};

} // namespace bomberman
