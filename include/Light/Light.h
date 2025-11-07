// Light.h
#pragma once
#include <concepts>
#include <glm/glm.hpp>
#include <glad/glad.h>

template<class P>
concept LightPropsType = requires(P p) {
    { p.AmbientColour }           -> std::convertible_to<glm::vec3>;
    { p.AmbientIntensity } -> std::convertible_to<float>;
    { p.DiffuseIntensity } -> std::convertible_to<float>;
};

template<class U>
concept UniformObjectsType = requires(U u) {
    { u.AmbientColourLocation }    -> std::convertible_to<GLint>;
    { u.AmbientIntensityLocation } -> std::convertible_to<GLint>;
    { u.DiffuseIntensityLocation } -> std::convertible_to<GLint>;
};

template<LightPropsType P, UniformObjectsType U>
class Light {
public:
    Light() = default;
    explicit Light(const P& in) : Props(in) {}
    virtual ~Light() = default;
    
    virtual void UseLight(const U& u) const {
        if (u.AmbientColourLocation    != -1) glUniform3f(u.AmbientColourLocation, Props.AmbientColour.r, Props.AmbientColour.g, Props.AmbientColour.b);
        if (u.AmbientIntensityLocation != -1) glUniform1f(u.AmbientIntensityLocation, Props.AmbientIntensity);
        if (u.DiffuseIntensityLocation != -1) glUniform1f(u.DiffuseIntensityLocation, Props.DiffuseIntensity);
    }
    
    virtual void UseLightDSA(GLuint program, const U& u) const {
        if (u.AmbientColourLocation    != -1) glProgramUniform3f(program, u.AmbientColourLocation, Props.AmbientColour.r, Props.AmbientColour.g, Props.AmbientColour.b);
        if (u.AmbientIntensityLocation != -1) glProgramUniform1f(program, u.AmbientIntensityLocation, Props.AmbientIntensity);
        if (u.DiffuseIntensityLocation != -1) glProgramUniform1f(program, u.DiffuseIntensityLocation, Props.DiffuseIntensity);
    }

    virtual void SetProperties(const P& p) { Props = p; }
    P&       GetProperties()       { return Props; }
    const P& GetProperties() const { return Props; }

protected:
    P Props{};
};
