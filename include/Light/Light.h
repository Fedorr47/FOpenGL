#pragma once

#include <concepts>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "Light/LightProperties.h"

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

class Light
{
public:
    Light() = default;
    virtual ~Light() = default;
    
    virtual std::unique_ptr<glm::mat4> CalculateLightTransform() const { return nullptr; }
    virtual std::vector<glm::mat4> CalculateLightTransformCube() const { return std::vector<glm::mat4>{};}
};

template<class Derived, template<class> class Traits>
class LightTypeMixin
{
public:
    using TraitsT      = Traits<Derived>;
    using PropsType    = typename TraitsT::Props;
    using UniformType  = typename TraitsT::UniformObjects;

    PropsType Properties{};

    PropsType&       GetLightProperties()       { return Properties; }
    const PropsType& GetLightProperties() const { return Properties; }

    virtual void SetProperties(const PropsType& p) { Properties = p; }
    
    virtual void UseLight(const UniformType& u) const
    {
        const auto& p = Properties;
        glUniform3f(u.Colour, p.Colour.r, p.Colour.g, p.Colour.b);
        glUniform1f(u.AmbientIntensity, p.AmbientIntensity);
        glUniform1f(u.DiffuseIntensity, p.DiffuseIntensity);
    }
};
