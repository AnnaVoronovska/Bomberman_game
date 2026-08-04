#pragma once
#include <random>
#include <chrono>
#include <algorithm>

// ============================================================
// Core.hpp
// Kleine, herbruikbare bouwstenen die de rest van de logic-library
// gebruikt: een 2D vector, een projectie-camera, en twee Singletons
// (Random en Stopwatch). Dit bestand bevat GEEN SFML: de logic-library
// mag nooit van de grafische laag afhangen.
// ============================================================

namespace bomberman {

// Eenvoudige 2D vector, gebruikt voor posities, groottes en richtingen.
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
};

// Projecteert genormaliseerde wereldcoördinaten ([-1,1] x [-1,1]) naar
// pixelcoördinaten op het venster. Volledig manueel geïmplementeerd,
// zonder SFML-utilities, zodat de logic-library SFML-onafhankelijk blijft.
class Camera {
public:
    Camera(double windowWidth, double windowHeight)
        : width_(windowWidth), height_(windowHeight) {}

    Vec2 worldToScreen(const Vec2& worldPos) const {
        double px = (worldPos.x + 1.0) / 2.0 * width_;
        double py = (worldPos.y + 1.0) / 2.0 * height_;
        return Vec2(px, py);
    }

    Vec2 worldToScreenSize(const Vec2& worldSize) const {
        return Vec2(worldSize.x / 2.0 * width_, worldSize.y / 2.0 * height_);
    }

private:
    double width_;
    double height_;
};

// Singleton rond een Mersenne Twister generator. De generator wordt als
// data-member bewaard zodat we telkens een NIEUWE waarde uit DEZELFDE
// generator trekken (i.p.v. elke keer een nieuwe generator te maken).
class Random {
public:
    static Random& instance();

    int getInt(int min, int max);   // inclusief bereik
    double getDouble01();           // [0, 1)

    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

private:
    Random();
    std::mt19937 engine_;
};

// Singleton die het tijdsverschil (deltaTime) tussen twee update-ticks
// bijhoudt. Wordt gebruikt door zowel de logic (beweging, bom-timers) als
// de representatie (animatie-timing), zonder ooit busy waiting te gebruiken.
class Stopwatch {
public:
    static Stopwatch& instance();

    void tick();                  // één keer per frame aanroepen
    double getDeltaTime() const;  // seconden sinds vorige tick
    double getTotalTime() const;  // seconden sinds het spel gestart is

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
