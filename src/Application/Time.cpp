
#include "Application/Time.h"
using namespace std::chrono;

void GameClock::reset() {
    last_ = clock::now();
    acc_ = 0.0;
    state_ = {};
    fpsAcc_ = 0.0; fpsCount_ = 0;
}

void GameClock::beginFrame() {
    auto now = clock::now();
    double dt = duration<double>(now - last_).count();
    last_ = now;

    state_.unscaledDelta = dt;
    double scaled = state_.paused ? 0.0 : dt * state_.timeScale;
    state_.deltaSec = scaled;
    state_.totalSec += scaled;
    acc_ += scaled;

    fpsAcc_ += dt; ++fpsCount_;
    if (fpsAcc_ >= 0.25) {
        state_.fps = fpsCount_ / fpsAcc_;
        fpsAcc_ = 0.0; fpsCount_ = 0;
    }
}
