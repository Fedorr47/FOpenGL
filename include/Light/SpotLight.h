#pragma once
#include "Light/Light.h" 
#include "Light/LightProperties.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

class SpotLight : public Light<SpotLightProperties, SpotLightUniformObjects>
{
public:
    using Base = Light<SpotLightProperties, SpotLightUniformObjects>;

    SpotLight() = default;

    explicit SpotLight(const SpotLightProperties& p) : Base(p) {
        updateDerived();
    }

    void SetProperties(const SpotLightProperties& p) override {
        Base::SetProperties(p);
        updateDerived();
    }

    void UseLight(const SpotLightUniformObjects& u) const override {
        Base::UseLight(u);
        
        if (u.Position  != (GLuint)-1) glUniform3f(u.Position,  Props.Position.x, Props.Position.y, Props.Position.z);
        if (u.Constant  != (GLuint)-1) glUniform1f(u.Constant,  Props.Constant);
        if (u.Linear    != (GLuint)-1) glUniform1f(u.Linear,    Props.Linear);
        if (u.Exponent  != (GLuint)-1) glUniform1f(u.Exponent,  Props.Exponent);
        
        const glm::vec3 d = hasDir_ ? normDir_ : glm::vec3(0.0f, -1.0f, 0.0f);
        if (u.Direction != (GLuint)-1) glUniform3f(u.Direction, d.x, d.y, d.z);
        if (u.Edge      != (GLuint)-1) glUniform1f(u.Edge,      Props.ProcEdge);
    }

private:
    void updateDerived() {
        Props.ProcEdge = std::cos(glm::radians(Props.Edge));
        const float len = glm::length(Props.Direction);
        hasDir_  = (len > 0.0f);
        normDir_ = hasDir_ ? (Props.Direction / len) : glm::vec3(0.0f);
    }
    
    glm::vec3 normDir_{0.0f};
    bool hasDir_{false};
};
