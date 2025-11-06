#pragma once
#include <concepts>
#include <glm/glm.hpp>
#include <GL/glew.h>

template<class P>
concept LightPropsType = requires(P p) {
    { p.Colour } -> std::convertible_to<glm::vec3>;
    { p.AmbientIntensity } -> std::convertible_to<float>;
    { p.DiffuseIntensity } -> std::convertible_to<float>;
};

template<class U>
concept UniformObjectsType = requires(U u) {
    { u.Colour } -> std::convertible_to<GLint>;
    { u.AmbientIntensity } -> std::convertible_to<GLint>;
    { u.DiffuseIntensity } -> std::convertible_to<GLint>;
};

template<LightPropsType P, UniformObjectsType U>
class Light {
public:
    Light() = default;
    explicit Light(const P& in) : Props(in) {}
    virtual ~Light() = default;

    virtual void UseLight(const U& u) const {
        glUniform3f(u.Colour, Props.Colour.r, Props.Colour.g, Props.Colour.b);
        glUniform1f(u.AmbientIntensity, Props.AmbientIntensity);
        glUniform1f(u.DiffuseIntensity, Props.DiffuseIntensity);
    }

    virtual void SetProperties(const P& p) { Props = p; }
    P&       GetProperties()       { return Props; }
    const P& GetProperties() const { return Props; }

protected:
    P Props{};
};
