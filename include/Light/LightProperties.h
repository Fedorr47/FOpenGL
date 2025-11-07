
#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

struct CommonLightProperties {
    glm::vec3 AmbientColour{1.0f,1.0f,1.0f};
    float AmbientIntensity{0.1f};
    float DiffuseIntensity{1.0f};
};

struct DirectionalLightProperties : CommonLightProperties {
    glm::vec3 Direction{0.0f,-1.0f,0.0f};
};

struct PointLightProperties : CommonLightProperties {
    glm::vec3 Position{0.0f,1.0f,0.0f};
    float Constant{1.0f};
    float Linear{0.09f};
    float Exponent{0.032f};
};

// --------------------- uniform locations  ---------------------
struct CommonLightUniformObjects {
    GLint AmbientColourLocation{-1};     // location(base.colour)
    GLint AmbientIntensityLocation{-1};  // location(base.ambientIntensity)
    GLint DiffuseIntensityLocation{-1};  // location(base.diffuseIntensity)
};

struct DirectionalLightUniformObjects : CommonLightUniformObjects {
    GLint DirectionLocation{-1};         // location(direction)
};

struct PointLightUniformObjects : CommonLightUniformObjects {
    GLint PositionLocation{-1};                 // location(position)
    GLint ConstantAttenuationLocation{-1};      // location(constant)
    GLint LinearAttenuationLocation{-1};        // location(linear)
    GLint AttenuationExponentLocation{-1};      // location(exponent)
};
