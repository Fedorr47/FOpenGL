#include <gtest/gtest.h>
#include "Application/Time.h"

TEST(Time, PauseZeroDelta)
{
    GameClock gameClock;
    gameClock.reset();
    gameClock.setPaused(true);
    gameClock.beginFrame();
    EXPECT_DOUBLE_EQ(gameClock.get().deltaSec, 0.0);
}

TEST(Time, RunFixedFromStepOnce)
{
    GameClock gameClock;
    gameClock.reset();
    gameClock.setPaused(true);
    gameClock.stepOnce(0.1);

    int ticks = 0;
    gameClock.runFixed(0.02, [&](double){ ++ticks; });
    EXPECT_EQ(ticks, 5);
}