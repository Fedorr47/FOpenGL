#pragma once
#include <memory>
#include <span>

#include "Light/Light.h"
#include "Light/LightProperties.h"

class Shader;

class PointLight
  : public Light<PointLightProperties, PointLightUniformObjects>
{
public:
    using Base = Light<PointLightProperties, PointLightUniformObjects>;
    PointLight() = default;
    explicit PointLight(const PointLightProperties& p) : Base(p) {}
    
    void UseLight(const PointLightUniformObjects& u) const override {
        Base::UseLight(u);
        glUniform3f(u.Position, Props.Position.x, Props.Position.y, Props.Position.z);
        glUniform1f(u.Constant, Props.Constant);
        glUniform1f(u.Linear,   Props.Linear);
        glUniform1f(u.Exponent, Props.Exponent);
    }
};