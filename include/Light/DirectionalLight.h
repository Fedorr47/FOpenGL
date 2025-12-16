#pragma once
#include "Light/Light.h"
#include "Light/LightProperties.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class DirectionalLight
  : public Light<DirectionalLightProperties, DirectionalLightUniformObjects>
{
public:
    using Base = Light<DirectionalLightProperties, DirectionalLightUniformObjects>;
    DirectionalLight() = default;
    explicit DirectionalLight(const DirectionalLightProperties& p) : Base(p){}

    void UseLight(const DirectionalLightUniformObjects& u) const override {
        Base::UseLight(u);
        glm::vec3 d = Props.Direction;
        // TODO: change it later to dependency values
        if (glm::length(d) > 0.0f) d = glm::normalize(d);
        glUniform3f(u.Direction, d.x, d.y, d.z);
    }

    void SetProperties(const DirectionalLightProperties& p) override {
        Base::SetProperties(p);
        if (glm::length(Props.Direction) > 0.0f)
            Props.Direction = glm::normalize(Props.Direction);
    }

    std::unique_ptr<glm::mat4> CalculateLightTransform() const
    {
        glm::vec3 dir = Props.Direction;
        if (glm::length(dir) <= 1e-6f)
        {
            dir = glm::vec3(0, -1, 0);
        }
        dir = glm::normalize(dir);
        
        const glm::vec3 center = glm::vec3(0.0f, 0.0f, -2.5f);
        
        const float distance = 40.0f;
        const glm::vec3 lightPos = center - dir * distance;

        const glm::mat4 view = glm::lookAt(lightPos, center, glm::vec3(0,1,0));
        
        const float half = 25.0f;
        const float zNear = 1.0f;
        const float zFar  = 120.0f;
        const glm::mat4 proj = glm::ortho(-half, half, -half, half, zNear, zFar);

        return std::make_unique<glm::mat4>(proj * view);
    }
};
