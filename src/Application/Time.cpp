
#include "Application/Time.h"
using namespace std::chrono;

void GameClock::resetTime() {
    last_ = clock::now();
    acc_ = 0.0;
    state_ = {};
    emaFps_ = 0.0;
}

void GameClock::beginFrame(std::optional<double> externalUnscaledDtSec)
{
    double deltaTime;
    if (externalUnscaledDtSec)
    {
        deltaTime = *externalUnscaledDtSec;
    }
    else
    {
        auto now = clock::now();
        deltaTime = std::chrono::duration<double>(now - last_).count();
        last_ = now;
    }
    
    deltaTime = std::clamp(deltaTime, 0.0, maxUnscaledFrameDeltaSec_);

    state_.unscaledDelta = deltaTime;
    state_.unscaledTotal += deltaTime;

    const double scaled = state_.paused ? 0.0 : deltaTime * state_.timeScale;
    state_.deltaSec  = scaled;
    state_.totalSec += scaled;
    acc_ += scaled;

    // FPS
    state_.rawFps = (deltaTime > 1e-9) ? (1.0 / deltaTime) : state_.rawFps;
    const double alpha = 0.10;
    if (emaFps_ <= 0.0)
    {
        emaFps_ = state_.rawFps;
    }
    else
    {
        emaFps_ = emaFps_ + alpha * (state_.rawFps - emaFps_);
    }
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
    fixedStep_ = dur{sec};
}
