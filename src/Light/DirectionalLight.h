#pragma once
#include "Light/Light.h"
#include "Light/LightProperties.h"

class DirectionalLight
  : public Light
  , public LightTypeMixin<DirectionalLight, LightTraits>
{
public:
    using Mixin     = LightTypeMixin<DirectionalLight, LightTraits>;
    using PropsType = typename Mixin::PropsType;
    using UniformT  = typename Mixin::UniformType;

    DirectionalLight() = default;
    DirectionalLight(const PropsType& inProps)
    {
        DirectionalLight::SetProperties(inProps);
    }
    
    void UseLight(const DirectionalLightUniformObjects& u) const override
    {
        Mixin::UseLight(u);
        
        glm::vec3 dir = Mixin::GetLightProperties().Direction;
        if (glm::length(dir) > 1e-12f)
        {
            dir = glm::normalize(dir);
        }
        else dir = glm::vec3(0.f, -1.f, 0.f);

        glUniform3f(u.Direction, dir.x, dir.y, dir.z);
    }

    void SetProperties(const DirectionalLightProperties& inProps) override
    {
        Mixin::SetProperties(inProps);
        
        auto& dir = Mixin::GetLightProperties().Direction;
        if (glm::length(dir) > 1e-12f)
        {
            dir = glm::normalize(dir);
        }
        else
        {
            dir = glm::vec3(0.f, -1.f, 0.f);
        }
    }

    std::unique_ptr<glm::mat4> CalculateLightTransform() const override
    {
        glm::vec3 dir = Mixin::GetLightProperties().Direction;
        if (glm::length(dir) <= 1e-12f)
        {
            dir = glm::vec3(0.f, -1.f, 0.f);
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
