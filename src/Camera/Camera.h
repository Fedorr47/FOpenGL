
#pragma once
#include <glm/glm.hpp>
#include <span>

class Camera {
public:
    Camera(
        glm::vec3 pos,
        glm::vec3 up,
        float yawDeg,
        float pitchDeg,
        float moveSpeed,
        float turnSpeed);

    void KeyControl(std::span<const bool> keys, float dt);
    void MouseControl(double deltaX, double deltaY, float dt);

    glm::mat4 GetViewMatrix() const;
    const glm::vec3& GetPosition() const { return position_; }
    const glm::vec3 GetDirection() const { return glm::normalize(front_); }

private:
    void UpdateVectors();

private:
    glm::vec3 position_;
    glm::vec3 worldUp_;
    glm::vec3 front_{0,0,-1}, right_{1,0,0}, up_{0,1,0};
    float yawDeg_;
    float pitchDeg_;
    float moveSpeed_;
    float turnSpeed_;
};
