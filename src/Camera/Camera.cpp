
#include "Camera/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <GLFW/glfw3.h>

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yawDeg, float pitchDeg, float moveSpeed, float turnSpeed)
: position_(pos), worldUp_(up), yawDeg_(yawDeg), pitchDeg_(pitchDeg), moveSpeed_(moveSpeed), turnSpeed_(turnSpeed)
{
    UpdateVectors();
}

void Camera::UpdateVectors()
{
    float yaw = glm::radians(yawDeg_);
    float pitch = glm::radians(pitchDeg_);
    glm::vec3 front;
    front.x = cosf(yaw) * cosf(pitch);
    front.y = sinf(pitch);
    front.z = sinf(yaw) * cosf(pitch);
    front_ = glm::normalize(front);
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_    = glm::normalize(glm::cross(right_, front_));
}

void Camera::KeyControl(std::span<const bool> keys, float dt)
{
    float vel = moveSpeed_ * dt;
    if (keys[GLFW_KEY_W]) position_ += front_ * vel;
    if (keys[GLFW_KEY_S]) position_ -= front_ * vel;
    if (keys[GLFW_KEY_A]) position_ -= right_ * vel;
    if (keys[GLFW_KEY_D]) position_ += right_ * vel;
    if (keys[GLFW_KEY_Q]) position_ -= up_ * vel;
    if (keys[GLFW_KEY_E]) position_ += up_ * vel;
}

void Camera::MouseControl(double dx, double dy, float dt)
{
    (void)dt;
    yawDeg_   += static_cast<float>(dx) * turnSpeed_;
    pitchDeg_ += static_cast<float>(dy) * turnSpeed_;
    pitchDeg_ = std::clamp(pitchDeg_, -89.0f, 89.0f);
    UpdateVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position_, position_ + front_, up_);
}
