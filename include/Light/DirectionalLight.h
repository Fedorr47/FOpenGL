
#pragma once
#include "Light/LightCommon.h"
#include "Rendering/Shader.h"
#include <glm/glm.hpp>
#include <glm/geometric.hpp>

class DirectionalLight {
public:
    DirectionalLight() = default;
    explicit DirectionalLight(const DirectionalLightProperties& p): props(p) {}

    void Apply(const Shader& sh) const {
        auto d = props.Direction;
        if (glm::length(d) > 0.0f) d = glm::normalize(d);
        glUniform3f(sh.DirLight.colour, props.Colour.r, props.Colour.g, props.Colour.b);
        glUniform1f(sh.DirLight.ambientIntensity, props.AmbientIntensity);
        glUniform1f(sh.DirLight.diffuseIntensity, props.DiffuseIntensity);
        glUniform3f(sh.DirLight.direction, d.x, d.y, d.z);
    }

    DirectionalLightProperties& Properties() { return props; }
    const DirectionalLightProperties& Properties() const { return props; }

private:
    DirectionalLightProperties props{};
};
