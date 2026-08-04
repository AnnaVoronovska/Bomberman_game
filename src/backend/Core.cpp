#include "Core.h"

namespace bomberman {

Random::Random() : engine_(std::random_device{}()) {}

Random& Random::instance() {
    static Random instance_;
    return instance_;
}

int Random::getInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(engine_);
}

double Random::getDouble01() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(engine_);
}

Stopwatch::Stopwatch() {
    start_ = std::chrono::steady_clock::now();
    last_ = start_;
}

Stopwatch& Stopwatch::instance() {
    static Stopwatch instance_;
    return instance_;
}

void Stopwatch::tick() {
    auto now = std::chrono::steady_clock::now();
    deltaTime_ = std::chrono::duration<double>(now - last_).count();
    totalTime_ = std::chrono::duration<double>(now - start_).count();
    last_ = now;
}

double Stopwatch::getDeltaTime() const { return deltaTime_; }
double Stopwatch::getTotalTime() const { return totalTime_; }

} // namespace bomberman
