// PointLight.h
#pragma once
#include "Light/Light.h"
#include "Light/LightProperties.h"

class PointLight
  : public Light<PointLightProperties, PointLightUniformObjects>
{
public:
    using Base = Light<PointLightProperties, PointLightUniformObjects>;
    PointLight() = default;
    explicit PointLight(const PointLightProperties& p) : Base(p) {}

    void UseLight(const PointLightUniformObjects& u) const override {
        Base::UseLight(u);
        if (u.PositionLocation            != -1) glUniform3f(u.PositionLocation, Props.Position.x, Props.Position.y, Props.Position.z);
        if (u.ConstantAttenuationLocation != -1) glUniform1f(u.ConstantAttenuationLocation, Props.Constant);
        if (u.LinearAttenuationLocation   != -1) glUniform1f(u.LinearAttenuationLocation,   Props.Linear);
        if (u.AttenuationExponentLocation != -1) glUniform1f(u.AttenuationExponentLocation, Props.Exponent);
    }

    void UseLightDSA(GLuint program, const PointLightUniformObjects& u) const override {
        Base::UseLightDSA(program, u);
        if (u.PositionLocation            != -1) glProgramUniform3f(program, u.PositionLocation, Props.Position.x, Props.Position.y, Props.Position.z);
        if (u.ConstantAttenuationLocation != -1) glProgramUniform1f(program, u.ConstantAttenuationLocation, Props.Constant);
        if (u.LinearAttenuationLocation   != -1) glProgramUniform1f(program, u.LinearAttenuationLocation,   Props.Linear);
        if (u.AttenuationExponentLocation != -1) glProgramUniform1f(program, u.AttenuationExponentLocation, Props.Exponent);
    }
};
