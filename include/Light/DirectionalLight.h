// DirectionalLight.h
#pragma once
#include "Light/Light.h"
#include "Light/LightProperties.h"
#include <glm/geometric.hpp>

class DirectionalLight
  : public Light<DirectionalLightProperties, DirectionalLightUniformObjects>
{
public:
    using Base = Light<DirectionalLightProperties, DirectionalLightUniformObjects>;
    DirectionalLight() = default;
    explicit DirectionalLight(const DirectionalLightProperties& p) : Base(p) {}
    
    void UseLight(const DirectionalLightUniformObjects& u) const override {
        Base::UseLight(u);
        glm::vec3 d = Props.Direction;
        if (glm::length(d) > 0.0f) d = glm::normalize(d);
        if (u.DirectionLocation != -1) glUniform3f(u.DirectionLocation, d.x, d.y, d.z);
    }

    void UseLightDSA(GLuint program, const DirectionalLightUniformObjects& u) const override {
        Base::UseLightDSA(program, u);
        glm::vec3 d = Props.Direction;
        if (glm::length(d) > 0.0f) d = glm::normalize(d);
        if (u.DirectionLocation != -1) glProgramUniform3f(program, u.DirectionLocation, d.x, d.y, d.z);
    }

    void SetProperties(const DirectionalLightProperties& p) override {
        Base::SetProperties(p);
        if (glm::length(Props.Direction) > 0.0f)
            Props.Direction = glm::normalize(Props.Direction);
    }
};
