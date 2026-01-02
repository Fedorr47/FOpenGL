#pragma once

#include <algorithm>
#include <chrono>
#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <limits>
#include <cmath>

struct TimeState
{
    // Game time (scaled, paused aware)
    double totalSec{0.0};
    double deltaSec{0.0};

    // Simulation input time (unscaled but CLAMPED, paused unaware)
    // Good for accumulation / fixed-step stability.
    double unscaledTotal{0.0};
    double unscaledDelta{0.0};

    // Real wall-clock time (UNCLAMPED)
    // Good for profiling, "honest" FPS, stats, logs.
    double wallTotalSec{0.0};
    double wallDeltaSec{0.0};

    // FPS
    double fps{0.0};     // EMA-smoothed
    double rawFps{0.0};  // instant

    float timeScale{1.0f};
    bool  paused{false};
};

struct FixedTickArgs
{
    std::chrono::duration<double> step;
    int tickIndex{0};
    std::chrono::duration<double> consumed{0.0};
};

enum class ControlFlow
{
    Continue,
    Break
};

template<class Callable>
concept FixedTickCallback =
    std::invocable<Callable&, std::chrono::duration<double>> ||
    std::invocable<Callable&, const FixedTickArgs&> ||
    std::invocable<Callable&>;

class GameClock
{
public:
    GameClock() = default;

    // Seconds per one tick of steady_clock::duration
    static double oneClockTickSec();

    void resetTime();
    void beginFrame(std::optional<double> externalWallDtSec = std::nullopt);

    void setPaused(bool paused) { state_.paused = paused; }
    void setTimeScale(float timeScale) { state_.timeScale = timeScale; }

    void setFixedStep(double stepSeconds);
    void setMaxFrameClamp(double maxUnscaledFrameDeltaSeconds);
    void setMaxCatchUp(double maxCatchUpSeconds);

    double getFixedStep() const { return fixedStep_.count(); }
    double getMaxFrameClamp() const { return maxUnscaledFrameDelta_.count(); }
    double getMaxCatchUp() const { return catchUpBudget_.count(); }

    const TimeState& getState() const { return state_; }

    // Manual injection into accumulator (testing/determinism)
    void stepOnce(double stepSeconds);

    template<FixedTickCallback Callback>
    int runFixed(Callback&& onFixedTick)
    {
        return runFixedInternal_(fixedStep_, std::numeric_limits<int>::max(), onFixedTick);
    }

    template<FixedTickCallback Callback>
    int runFixedForStep(std::chrono::duration<double> step, Callback&& onFixedTick)
    {
        return runFixedInternal_(step, std::numeric_limits<int>::max(), onFixedTick);
    }

    template<FixedTickCallback Callback>
    int runFixedCapped(std::chrono::duration<double> step, int maxTicks, Callback&& onFixedTick)
    {
        const int safeMaxTicks = std::max(0, maxTicks);
        return runFixedInternal_(step, safeMaxTicks, onFixedTick);
    }

    // Classic fixed-step interpolation alpha for rendering
    // alpha = accumulator / fixedStep in [0..1]
    double getFixedStepInterpolationAlpha() const;

private:
    using SteadyClock_ = std::chrono::steady_clock;
    using TimePoint_   = SteadyClock_::time_point;
    using Seconds_     = std::chrono::duration<double>;

private:
    template<class Callback>
    static ControlFlow normalizeControlFlow_(ControlFlow flow) { return flow; }

    template<class Callback>
    static ControlFlow normalizeControlFlow_(...) { return ControlFlow::Continue; } // for void / other returns

    template<class Callback>
    static ControlFlow invokeCallback_(Callback& callback, const Seconds_ step, const FixedTickArgs& args)
    {
        if constexpr (std::invocable<Callback&, Seconds_>)
        {
            if constexpr (std::is_same_v<std::invoke_result_t<Callback&, Seconds_>, ControlFlow>)
            {
                return std::invoke(callback, step);
            }
            else
            {
                std::invoke(callback, step);
                return ControlFlow::Continue;
            }
        }
        else if constexpr (std::invocable<Callback&, const FixedTickArgs&>)
        {
            if constexpr (std::is_same_v<std::invoke_result_t<Callback&, const FixedTickArgs&>, ControlFlow>)
            {
                return std::invoke(callback, args);
            }
            else
            {
                std::invoke(callback, args);
                return ControlFlow::Continue;
            }
        }
        else
        {
            if constexpr (std::is_same_v<std::invoke_result_t<Callback&>, ControlFlow>)
            {
                return std::invoke(callback);
            }
            else
            {
                std::invoke(callback);
                return ControlFlow::Continue;
            }
        }
    }

    template<FixedTickCallback Callback>
    int runFixedInternal_(Seconds_ step, int maxTicks, Callback&& onFixedTick)
    {
        // Named variable is lvalue => we intentionally call callback via lvalue reference.
        auto& callbackReference = onFixedTick;

        const Seconds_ availableCatchUp = std::min(accumulatedScaledTime_, catchUpBudget_);
        Seconds_ remainingBudget = availableCatchUp;

        int executedTicks = 0;

        FixedTickArgs args{};
        args.step = step;
        args.tickIndex = 0;
        args.consumed = Seconds_{0.0};

        // Small epsilon to avoid missing a tick due to floating precision
        const Seconds_ comparisonEpsilon{1e-12};

        while ((remainingBudget + comparisonEpsilon) >= step && executedTicks < maxTicks)
        {
            if (invokeCallback_(callbackReference, step, args) == ControlFlow::Break)
            {
                break;
            }

            remainingBudget        -= step;
            accumulatedScaledTime_ -= step;

            args.consumed += step;
            ++args.tickIndex;
            ++executedTicks;
        }

        return executedTicks;
    }

private:
    TimePoint_ lastFrameTimePoint_{SteadyClock_::now()};

    // Accumulator uses SCALED dt (pause + timeScale)
    Seconds_ accumulatedScaledTime_{0.0};

    TimeState state_{};

    Seconds_ fixedStep_{1.0 / 60.0};
    Seconds_ maxUnscaledFrameDelta_{0.100};
    Seconds_ catchUpBudget_{0.250};

    double fpsEma_{0.0};
};
