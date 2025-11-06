
#pragma once
#include "Light/LightCommon.h"
#include "Rendering/Shader.h"

class PointLight {
public:
    PointLight() = default;
    explicit PointLight(const PointLightProperties& p): props(p) {}

    void Apply(const Shader& sh, int index) const {
        const auto& u = sh.PointLights[index];
        glUniform3f(u.colour, props.Colour.r, props.Colour.g, props.Colour.b);
        glUniform1f(u.ambientIntensity, props.AmbientIntensity);
        glUniform1f(u.diffuseIntensity, props.DiffuseIntensity);
        glUniform3f(u.position, props.Position.x, props.Position.y, props.Position.z);
        glUniform1f(u.constant, props.Constant);
        glUniform1f(u.linear,   props.Linear);
        glUniform1f(u.exponent, props.Exponent);
    }

    PointLightProperties& Properties() { return props; }

private:
    PointLightProperties props{};
};
