#include <gtest/gtest.h>
#include <chrono>
#include "Application/Time.h"

using namespace std::chrono;
using namespace std::chrono_literals;

TEST(Time, PauseZeroDelta)
{
    GameClock gameClock;
    gameClock.reset();
    gameClock.setPaused(true);
    gameClock.beginFrame(16ms);
    const auto& state = gameClock.get();
    EXPECT_DOUBLE_EQ(state.deltaSec, 0.0);
    EXPECT_NEAR(state.unscaledDelta, 0.016, 1e-9);
}

TEST(GameClock, FrameClampAppliesToUnscaled) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.setMaxFrameClamp(20ms);
    gameClock.beginFrame(500ms);
    EXPECT_NEAR(gameClock.get().unscaledDelta, 0.02, 1e-9);
}

TEST(GameClock, TimeScaleAffectsScaledDeltaOnly) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.setTimeScale(2.0f);
    gameClock.beginFrame(10ms);
    const auto& st = gameClock.get();
    EXPECT_NEAR(st.unscaledDelta, 0.01, 1e-9);
    EXPECT_NEAR(st.deltaSec,      0.02, 1e-9);
}

TEST(GameClock, StepOnceAndRunFixedWithDuration) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.stepOnce(50ms);
    int ticks = 0;
    gameClock.runFixedForStep(10ms, [&](std::chrono::duration<double> dt){
        EXPECT_NEAR(dt.count(), 0.01, 1e-12);
        ++ticks;
    });
    EXPECT_EQ(ticks, 5);
}

TEST(GameClock, RunFixedWithArgsProvidesIndexAndConsumed) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.stepOnce(50ms);
    int lastIdx = -1; double consumed = -1.0;
    int ticks = gameClock.runFixedForStep(10ms, [&](const FixedTickArgs& a){
        lastIdx  = a.tickIndex;
        consumed = a.consumedSec;
    });
    EXPECT_EQ(ticks, 5);
    EXPECT_EQ(lastIdx, 5);
    EXPECT_NEAR(consumed, 0.05, 1e-12);
}

TEST(GameClock, RunFixedNoArgWorks) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.stepOnce(30ms);
    int cnt = 0;
    gameClock.runFixedForStep(10ms, [&]{ ++cnt; });
    EXPECT_EQ(cnt, 3);
}

TEST(GameClock, CatchUpBudgetLimitsTicks) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.setMaxFrameClamp(1s);
    gameClock.setMaxCatchUp(50ms);
    gameClock.beginFrame(0.5s);
    int ticks = gameClock.runFixedForStep(10ms, []{});
    EXPECT_EQ(ticks, 5);
}

TEST(GameClock, RunFixedCappedLimitsTicksCount) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.stepOnce(200ms);
    int ticks = gameClock.runFixedCapped(10ms, 3, []{});
    EXPECT_EQ(ticks, 3);
}

TEST(GameClock, SetFixedStepClampedToSaneMin) {
    GameClock gameClock;
    gameClock.reset();
    gameClock.setFixedStep(std::numeric_limits<double>::quiet_NaN());
    EXPECT_GE(gameClock.getFixedStep(), 1e-6);
    gameClock.setFixedStep(0.0);
    EXPECT_GE(gameClock.getFixedStep(), 1e-6);
}

TEST(GameClock, EmaFpsConvergesNear60) {
    GameClock c; 
    c.reset();

    // 30 FPS
    for (int i = 0; i < 30; ++i)
        c.beginFrame(nanoseconds{33333333});

    // 60 FPS
    for (int i = 0; i < 90; ++i)
        c.beginFrame(nanoseconds{16666667});

    EXPECT_NEAR(c.get().fps, 60.0, 5.0);
}