#pragma once
#include <memory>
#include <span>

#include "Light/Light.h"
#include "Light/LightProperties.h"
#include "Rendering/Shadow/OmniShadowMap.h"

class Shader;

class PointLight
  : public Light<PointLightProperties, PointLightUniformObjects>
{
public:
    using Base = Light<PointLightProperties, PointLightUniformObjects>;
    PointLight() = default;
    explicit PointLight(const PointLightProperties& p) : Base(p){ }
    
    void UseLight(const PointLightUniformObjects& u) const override {
        Base::UseLight(u);
        glUniform3f(u.Position, Props.Position.x, Props.Position.y, Props.Position.z);
        glUniform1f(u.Constant, Props.Constant);
        glUniform1f(u.Linear,   Props.Linear);
        glUniform1f(u.Exponent, Props.Exponent);
    }

    void SetProperties(const PointLightProperties& p) override
    {
        float aspect =
            static_cast<float>(Props.shadowMapPtr->GetShadowWidth()) /
                static_cast<float>(Props.shadowMapPtr->GetShadowHeight());
        Props.lightProj = glm::perspective(glm::radians(90.0f), aspect, Props.nearPlane, Props.farPlane);
    }

    float GetFarPlane() const { return Props.farPlane; }
    
    std::vector<glm::mat4> CalculateLightTransformCube() const override
    {
        std::vector<glm::mat4> lightMatrices;

        lightMatrices.push_back(Props.lightProj * glm::lookAt(
            Props.Position,
            Props.Position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        lightMatrices.push_back(Props.lightProj * glm::lookAt(
            Props.Position,
            Props.Position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        
        lightMatrices.push_back(Props.lightProj * glm::lookAt(
            Props.Position,
            Props.Position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        lightMatrices.push_back(Props.lightProj * glm::lookAt(
            Props.Position,
            Props.Position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));

        lightMatrices.push_back(Props.lightProj * glm::lookAt(
            Props.Position,
            Props.Position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        lightMatrices.push_back(Props.lightProj * glm::lookAt(
            Props.Position,
            Props.Position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        return lightMatrices;
    }
};