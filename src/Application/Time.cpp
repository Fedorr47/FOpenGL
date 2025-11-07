
#include "Application/Time.h"
using namespace std::chrono;

void GameClock::reset() {
    last_ = clock::now();
    acc_ = 0.0;
    state_ = {};
    emaFps_ = 0.0;
}

void GameClock::beginFrame() {
    auto now = clock::now();
    double dt = std::chrono::duration<double>(now - last_).count();
    last_ = now;

    dt = std::clamp(dt, 0.0, maxUnscaledFrameDeltaSec_);
    state_.unscaledDelta = dt;
    state_.unscaledTotal += dt;

    const double scaled = state_.paused ? 0.0 : dt * state_.timeScale;
    state_.deltaSec  = scaled;
    state_.totalSec += scaled;
    acc_ += scaled;

    state_.rawFps = (dt > 1e-9) ? (1.0 / dt) : state_.rawFps;
    const double alpha = 0.10;
    emaFps_ = (emaFps_ <= 0.0) ? state_.rawFps : (emaFps_ + alpha * (state_.rawFps - emaFps_));
    state_.fps = emaFps_;
}

void GameClock::beginFrame(std::chrono::duration<double> externalUnscaledDt) {
    double dt = std::clamp(externalUnscaledDt.count(), 0.0, maxUnscaledFrameDeltaSec_);
    state_.unscaledDelta = dt;
    state_.unscaledTotal += dt;

    const double scaled = state_.paused ? 0.0 : dt * state_.timeScale;
    state_.deltaSec  = scaled;
    state_.totalSec += scaled;
    acc_ += scaled;

    state_.rawFps = (dt > 1e-9) ? (1.0 / dt) : state_.rawFps;
    const double alpha = 0.10;
    emaFps_ = (emaFps_ <= 0.0) ? state_.rawFps : (emaFps_ + alpha * (state_.rawFps - emaFps_));
    state_.fps = emaFps_;
}

void GameClock::setFixedStep(double sec)
{
    constexpr double kSaneMin = 1e-6;
    const double minStep = std::max(oneClockTickSec(), kSaneMin);

    if (!std::isfinite(sec))
    {
        sec = minStep;
    }
    
    sec = std::max(sec, minStep);
    fixedStep_ = sec;
}
