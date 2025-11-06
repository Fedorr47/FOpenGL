
#pragma once
#include <glm/glm.hpp>

struct CommonLightProperties {
    glm::vec3 Colour{1.0f,1.0f,1.0f};
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
