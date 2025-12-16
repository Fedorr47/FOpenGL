#pragma once
#include <glm/glm.hpp>

#include <memory>

struct CommonLightProperties {
    glm::vec3 Colour{1.0f,1.0f,1.0f};
    float AmbientIntensity{0.1f};
    float DiffuseIntensity{1.0f};
    glm::mat4 lightProj = glm::mat4(1.0f);
    
    std::shared_ptr<ShadowMap> shadowMapPtr = nullptr;

    CommonLightProperties()
    {
        shadowMapPtr = std::make_shared<ShadowMap>();
    }
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

struct SpotLightProperties : PointLightProperties {
    glm::vec3 Direction{0.0f,1.0f,0.0f};
    GLfloat Edge{1.0f};
    GLfloat ProcEdge{1.0f};
};

// -------- locations uniform-fields in a shader --------
struct CommonLightUniformObjects {
    GLint Colour{-1};
    GLint AmbientIntensity{-1};
    GLint DiffuseIntensity{-1};
};

struct DirectionalLightUniformObjects : CommonLightUniformObjects {
    GLint Direction{-1};
};

struct PointLightUniformObjects : CommonLightUniformObjects {
    GLint Position{-1};
    GLint Constant{-1};
    GLint Linear{-1};
    GLint Exponent{-1};
};

struct SpotLightUniformObjects : PointLightUniformObjects {
    GLint Direction{-1};
    GLint ProcEdge{-1};
    GLint Edge{-1};
};