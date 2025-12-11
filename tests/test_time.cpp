#include <gtest/gtest.h>
#include "Application/Time.h"

TEST(Time, PauseZeroDelta)
{
    GameClock gameClock;
    gameClock.resetTime();
    gameClock.setPaused(true);
    gameClock.beginFrame();
    EXPECT_DOUBLE_EQ(gameClock.getState().deltaSec, 0.0);
}

TEST(Time, RunFixedFromStepOnce)
{
    GameClock gameClock;
    gameClock.resetTime();
    gameClock.setPaused(true);
    gameClock.stepOnce(0.1);

    int ticks = 0;
    gameClock.runFixed([&](std::chrono::duration<double> dt)
    {
        ++ticks;
    });
    EXPECT_EQ(ticks, 6);
}