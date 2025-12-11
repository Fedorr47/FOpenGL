#include <gtest/gtest.h>
#include "Camera/Camera.h"

TEST(Camera, CameraInit)
{
    std::unique_ptr<Camera> camera = std::make_unique<Camera>(
        glm::vec3(0.0f, 1.0f, 5.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, 0.0f, 5.0f, 0.1f);

    EXPECT_NE(camera, nullptr);
}

/*
TEST(Camera, MoveAround)
{
    
}
*/