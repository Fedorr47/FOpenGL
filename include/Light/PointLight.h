#pragma once
#include <memory>

#include <glm/glm.hpp>
#include "Light/Light.h"
#include "Light/LightProperties.h"
#include "Rendering/Shadow/OmniShadowMap.h"

class Shader;

template<class Derived>
class PointLightBase
    : public Light
    , public LightTypeMixin<Derived, LightTraits>
{
public:
    using Mixin     = LightTypeMixin<Derived, LightTraits>;
    using PropsType = typename Mixin::PropsType;
    using UniformType  = typename Mixin::UniformType;
    
    PointLightBase() = default;
    PointLightBase(const PropsType& inProps)
    {
        SetProperties(inProps);
    }
    
    void UseLight(const UniformType& u) const override
    {
        Mixin::UseLight(u);

        const auto& p = Mixin::GetLightProperties();
        glUniform3f(u.Position, p.Position.x, p.Position.y, p.Position.z);
        glUniform1f(u.Constant, p.Constant);
        glUniform1f(u.Linear,   p.Linear);
        glUniform1f(u.Exponent, p.Exponent);
    }

    const CommonLightProperties* GetProperties() const
    {
        return &Mixin::GetLightProperties();
    }

    void SetProperties(const PropsType& p) override
    {
        Mixin::SetProperties(p);
        auto& Props = Mixin::GetLightProperties();
        
        float aspect = 1.0f;

        if (Props.shadowMapPtr)
        {
            const float w = float(Props.shadowMapPtr->GetShadowWidth());
            const float h = float(Props.shadowMapPtr->GetShadowHeight());
            if (w > 0.0f && h > 0.0f)
                aspect = w / h;
        }

        Props.lightProj = glm::perspective(glm::radians(90.0f), aspect, Props.nearPlane, Props.farPlane);
    }

    float GetFarPlane() const { return Mixin::GetLightProperties().farPlane; }
    
    std::vector<glm::mat4> CalculateLightTransformCube() const override
    {
        const auto& p = Mixin::GetLightProperties();
        const glm::vec3& pos = p.Position;

        return {
            // +X / -X
            p.lightProj * glm::lookAt(pos, pos + glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)),
            p.lightProj * glm::lookAt(pos, pos + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),

            // +Y / -Y
            p.lightProj * glm::lookAt(pos, pos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
            p.lightProj * glm::lookAt(pos, pos + glm::vec3(0,-1, 0), glm::vec3(0, 0,-1)),

            // +Z / -Z
            p.lightProj * glm::lookAt(pos, pos + glm::vec3(0, 0, 1), glm::vec3(0,-1, 0)),
            p.lightProj * glm::lookAt(pos, pos + glm::vec3(0, 0,-1), glm::vec3(0,-1, 0))
        };
    }
};

class PointLight : public PointLightBase<PointLight>
{
public:
    using PointLightBase<PointLight>::PointLightBase;
};