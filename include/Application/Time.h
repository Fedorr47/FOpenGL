#pragma once
#include <chrono>
#include <algorithm>
#include <concepts>
#include <optional>

struct TimeState {
    double totalSec{0.0};
    double deltaSec{0.0};
    
    double unscaledTotal{0.0};
    double unscaledDelta{0.0};
    
    double fps{0.0};
    double rawFps{0.0};
    
    float  timeScale{1.0f};
    bool   paused{false};
};

struct FixedTickArgs {
    std::chrono::duration<double> dt;
    int   tickIndex;
    double consumedSec;
};

enum class ControlFlow
{
    Continue,
    Break
};

template<class F>
concept FixedTickFn =
    std::invocable<F, std::chrono::duration<double>> ||
    std::invocable<F, const FixedTickArgs&> ||
    std::invocable<F>; 

class GameClock {
    using clock      = std::chrono::steady_clock;
    using dur   = std::chrono::duration<double>;
public:

    GameClock() = default;

    static double oneClockTickSec() {
        return std::chrono::duration<double>(typename clock::duration(1)).count();
    }
    
    void resetTime();
    void beginFrame(std::optional<double> externalUnscaledDtSec = std::nullopt);
    void setPaused(bool bPaused) { state_.paused = bPaused; }
    void setFixedStep(double sec);
    void setTimeScale(float inTimeScale) { state_.timeScale = inTimeScale; }
    void setMaxFrameClamp(double sec) { maxUnscaledFrameDeltaSec_ = std::max(1e-6, sec); }
    void setMaxCatchUp(double sec)    { catchUpBudgetSec_    = std::max(0.0,  sec); }
    
    double getFixedStep()     const { return fixedStep_; }
    double getMaxFrameClamp() const { return maxUnscaledFrameDeltaSec_; }
    double getMaxCatchUp()    const { return catchUpBudgetSec_; }
    
    const TimeState& getState() const { return state_; }

    void stepOnce(double stepSec) { acc_ += std::max(0.0, stepSec);}
    template<FixedTickFn F>
    int runFixed(F&& onFixedTick) { return runFixedForStep(fixedStep_, std::forward<F>(onFixedTick)); }
    template<FixedTickFn F>
    int runFixedForStep(std::chrono::duration<double> step, F&& onFixedTick);
    template<FixedTickFn F>
    int runFixedCapped(std::chrono::duration<double> step, int maxTicks, F&& onFixedTick);
    
private:
    clock::time_point last_{clock::now()};
    double acc_{0.0};
    TimeState state_{};
    
    double fixedStep_{1.0/60.0};
    double maxUnscaledFrameDeltaSec_{0.100};
    double catchUpBudgetSec_{0.250};

    double emaFps_{0.0};
};

template<FixedTickFn F>
int GameClock::runFixedForStep(std::chrono::duration<double> step, F&& onFixedTick) {
    const double stepSec = step.count();
    double budget = std::min(acc_, catchUpBudgetSec_);
    int ticks = 0;

    FixedTickArgs args{step, /*tickIndex=*/0, /*consumedSec=*/0.0};

    while (budget + 1e-12 >= stepSec) {
        if constexpr (std::invocable<F, std::chrono::duration<double>>)
        {
            std::invoke(onFixedTick, step);
        }
        else if constexpr (std::invocable<F, const FixedTickArgs&>)
        {
            std::invoke(onFixedTick, args);
        }
        else
        {
            std::invoke(onFixedTick);
        }

        budget   -= stepSec;
        acc_     -= stepSec;
        args.consumedSec += stepSec;
        ++args.tickIndex;
        ++ticks;
    }
    return ticks;
}

template<FixedTickFn F>
int GameClock::runFixedCapped(std::chrono::duration<double> step, int maxTicks, F&& onFixedTick) {
    const double stepSec = step.count();
    double budget = std::min(acc_, catchUpBudgetSec_);
    int ticks = 0;

    FixedTickArgs args{step, /*tickIndex=*/0, /*consumedSec=*/0.0};

    while (budget + 1e-12 >= stepSec && ticks < maxTicks) {
        if constexpr (std::invocable<F, std::chrono::duration<double>>)
        {
            std::invoke(onFixedTick, step);
        }
        else if constexpr (std::invocable<F, const FixedTickArgs&>)
        {
            std::invoke(onFixedTick, args);
        }
        else
        {
            std::invoke(onFixedTick);
        }

        budget   -= stepSec;
        acc_     -= stepSec;
        args.consumedSec += stepSec;
        ++args.tickIndex;
        ++ticks;
    }
    return ticks;
}
