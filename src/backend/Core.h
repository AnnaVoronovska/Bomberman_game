#pragma once
#include <algorithm>
#include <chrono>
#include <random>

// ============================================================
// Core.h
// Kleine, herbruikbare bouwstenen die de rest van de logic-library
// gebruikt: een 2D vector, een projectie-camera, en twee Singletons
// (Random en Stopwatch). Dit bestand bevat GEEN SFML: de logic-library
// mag nooit van de grafische laag afhangen.
// ============================================================

namespace bomberman {

/**
 * @brief Eenvoudige 2D vector, gebruikt voor posities, groottes en richtingen.
 */
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2& operator+=(const Vec2& o) {
        x += o.x;
        y += o.y;
        return *this;
    }
};

/**
 * @brief Projecteert genormaliseerde wereldcoördinaten ([-1,1] x [-1,1]) naar
 * pixelcoördinaten op het venster. Volledig manueel geïmplementeerd,
 * zonder SFML-utilities, zodat de logic-library SFML-onafhankelijk blijft.
 */
class Camera {
public:
    /// @brief Maakt een Camera aan voor een venster van gegeven grootte (pixels).
    Camera(double windowWidth, double windowHeight) : width_(windowWidth), height_(windowHeight) {}

    /// @brief Zet een wereldpositie om naar een pixelpositie op het scherm.
    Vec2 worldToScreen(const Vec2& worldPos) const {
        double px = (worldPos.x + 1.0) / 2.0 * width_;
        double py = (worldPos.y + 1.0) / 2.0 * height_;
        return Vec2(px, py);
    }

    /// @brief Zet een wereldgrootte om naar een pixelgrootte (voor sprite-dimensies).
    Vec2 worldToScreenSize(const Vec2& worldSize) const {
        return Vec2(worldSize.x / 2.0 * width_, worldSize.y / 2.0 * height_);
    }

private:
    double width_;
    double height_;
};

/**
 * @brief Singleton rond een Mersenne Twister generator. De generator wordt als
 * data-member bewaard zodat we telkens een NIEUWE waarde uit DEZELFDE
 * generator trekken (i.p.v. elke keer een nieuwe generator te maken).
 */
class Random {
public:
    /// @brief Geeft de enige instantie van de Random singleton terug.
    static Random& instance();

    /// @brief Geeft een willekeurig geheel getal terug, inclusief [min, max].
    int getInt(int min, int max); // inclusief bereik
    /// @brief Geeft een willekeurige double terug in het interval [0, 1).
    double getDouble01();         // [0, 1)

    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

private:
    Random();
    std::mt19937 engine_;
};

/**
 * @brief Singleton die het tijdsverschil (deltaTime) tussen twee update-ticks
 * bijhoudt. Wordt gebruikt door zowel de logic (beweging, bom-timers) als
 * de representatie (animatie-timing), zonder ooit busy waiting te gebruiken.
 */
class Stopwatch {
public:
    /// @brief Geeft de enige instantie van de Stopwatch singleton terug.
    static Stopwatch& instance();

    /// @brief Registreert een nieuwe tick; moet één keer per frame aangeroepen worden.
    void tick();                 // één keer per frame aanroepen
    /// @brief Geeft het aantal seconden sinds de vorige tick terug.
    double getDeltaTime() const; // seconden sinds vorige tick
    /// @brief Geeft het totaal aantal seconden sinds de start van het spel terug.
    double getTotalTime() const; // seconden sinds het spel gestart is

    Stopwatch(const Stopwatch&) = delete;
    Stopwatch& operator=(const Stopwatch&) = delete;

private:
    Stopwatch();
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point last_;
    double deltaTime_ = 0.0;
    double totalTime_ = 0.0;
};

} // namespace bomberman