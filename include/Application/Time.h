
#pragma once
#include <chrono>

struct TimeState {
    double totalSec{0.0};
    double deltaSec{0.0};
    double unscaledDelta{0.0};
    double fps{0.0};
    float  timeScale{1.0f};
    bool   paused{false};
};

class GameClock {
    using clock = std::chrono::steady_clock;
public:
    void reset();
    void beginFrame();
    template<class Fn> void runFixed(double step, Fn&& cb);
    void setPaused(bool p) { state_.paused = p; }
    void setTimeScale(float s) { state_.timeScale = s; }
    void stepOnce(double step) { if (state_.paused) acc_ += step; }
    const TimeState& get() const { return state_; }
private:
    clock::time_point last_{clock::now()};
    double acc_{0.0};
    TimeState state_{};
    double fpsAcc_{0.0};
    int fpsCount_{0};
};

template<class Fn>
void GameClock::runFixed(double step, Fn&& cb) {
    const double maxCatchUp = 0.25;
    double budget = std::min(acc_, maxCatchUp);
    while (budget >= step) { cb(step); budget -= step; acc_ -= step; }
}
