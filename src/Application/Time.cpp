// GameClock.cpp
#include "Application/Time.h"

double GameClock::oneClockTickSec()
{
    // duration{1} means "one tick" of steady_clock's native duration
    return Seconds_{SteadyClock_::duration{1}}.count();
}

void GameClock::resetTime()
{
    lastFrameTimePoint_ = SteadyClock_::now();
    accumulatedScaledTime_ = Seconds_{0.0};
    state_ = {};
    fpsEma_ = 0.0;
}

void GameClock::beginFrame(std::optional<double> externalWallDtSec)
{
    // 1) Measure REAL wall delta (unclamped)
    double wallDeltaSecondsReal = 0.0;

    if (externalWallDtSec.has_value())
    {
        wallDeltaSecondsReal = *externalWallDtSec;
    }
    else
    {
        const TimePoint_ now = SteadyClock_::now();
        wallDeltaSecondsReal = Seconds_{now - lastFrameTimePoint_}.count();
        lastFrameTimePoint_ = now;
    }

    if (!std::isfinite(wallDeltaSecondsReal) || wallDeltaSecondsReal < 0.0)
    {
        wallDeltaSecondsReal = 0.0;
    }

    state_.wallDeltaSec = wallDeltaSecondsReal;
    state_.wallTotalSec += wallDeltaSecondsReal;

    // 2) Clamp dt for simulation stability (anti spiral-of-death)
    const double unscaledDeltaSecondsClamped =
        std::clamp(wallDeltaSecondsReal, 0.0, maxUnscaledFrameDelta_.count());

    state_.unscaledDelta = unscaledDeltaSecondsClamped;
    state_.unscaledTotal += unscaledDeltaSecondsClamped;

    // 3) Game time (pause + timeScale)
    const double scaledDeltaSeconds =
        state_.paused ? 0.0 : (unscaledDeltaSecondsClamped * static_cast<double>(state_.timeScale));

    state_.deltaSec = scaledDeltaSeconds;
    state_.totalSec += scaledDeltaSeconds;

    // Accumulate for fixed-step updates
    if (scaledDeltaSeconds > 0.0)
    {
        accumulatedScaledTime_ += Seconds_{scaledDeltaSeconds};
    }

    // 4) FPS: use REAL dt (honest)
    if (wallDeltaSecondsReal > 1e-9)
    {
        state_.rawFps = 1.0 / wallDeltaSecondsReal;
    }

    constexpr double fpsEmaAlpha = 0.10;
    if (fpsEma_ <= 0.0)
    {
        fpsEma_ = state_.rawFps;
    }
    else
    {
        fpsEma_ = fpsEma_ + fpsEmaAlpha * (state_.rawFps - fpsEma_);
    }

    state_.fps = fpsEma_;
}

void GameClock::setFixedStep(double stepSeconds)
{
    constexpr double saneMinimumSeconds = 1e-6;
    const double minimumStepSeconds = std::max(oneClockTickSec(), saneMinimumSeconds);

    if (!std::isfinite(stepSeconds))
    {
        stepSeconds = minimumStepSeconds;
    }

    stepSeconds = std::max(stepSeconds, minimumStepSeconds);
    fixedStep_ = Seconds_{stepSeconds};
}

void GameClock::setMaxFrameClamp(double maxUnscaledFrameDeltaSeconds)
{
    const double safeValue = std::max(1e-6, maxUnscaledFrameDeltaSeconds);
    maxUnscaledFrameDelta_ = Seconds_{safeValue};
}

void GameClock::setMaxCatchUp(double maxCatchUpSeconds)
{
    const double safeValue = std::max(0.0, maxCatchUpSeconds);
    catchUpBudget_ = Seconds_{safeValue};
}

void GameClock::stepOnce(double stepSeconds)
{
    if (std::isfinite(stepSeconds) && stepSeconds > 0.0)
    {
        accumulatedScaledTime_ += Seconds_{stepSeconds};
    }
}

double GameClock::getFixedStepInterpolationAlpha() const
{
    const double fixedStepSeconds = fixedStep_.count();
    if (fixedStepSeconds <= 0.0)
    {
        return 0.0;
    }

    const double alpha = accumulatedScaledTime_.count() / fixedStepSeconds;
    return std::clamp(alpha, 0.0, 1.0);
}
