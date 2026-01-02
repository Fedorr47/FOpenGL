#pragma once
#include <string>
#include <filesystem>
#include <array>
#include <type_traits>
#include <GL/glew.h>

#include "Light/LightProperties.h"

class Shader {
public:
    Shader();
    ~Shader();
    
    bool CreateFromFiles(
        const std::filesystem::path& vertexShader,
        const std::filesystem::path& fragmentShader,
        const std::filesystem::path& geometryShader = {});
    
    bool CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader);
    bool CreateGeometryFromFile(const std::filesystem::path& geometryShader);

    void Use() const;

    void SetTexture(GLuint textureUnit);
    void SetDirectionalShadowMap(GLuint textureUnit);
    void SetDirectionalLightTransform(const glm::mat4* lTransform);

    void SetUniformModel(GLint uniformModel) { uniModel_ = uniformModel; }
    
    void SetLightMatrices(const std::vector<glm::mat4>& lightMatrices);

    void Clear();

    GLint GetUniformModel() const { return uniModel_; }
    GLint GetUniformView() const { return uniView_; }
    GLint GetUniformProj() const { return uniProj_; }
    GLint GetUniformEyePos() const { return uniEyePos_; }
    GLint GetUniformSpecularIntensity() const { return uniSpecularIntensity_; }
    GLint GetUniformShininess() const { return uniShininess_; }
    GLint GetOmniLightPos() const { return uniOmniLightPos_; }
    GLint GetOmniFarPlane() const { return uniOmniFarPlane_; }

    template<typename T>
    T* GetUniformArray() {
        if constexpr (std::is_same_v<T, PointLightUniformObjects>) {
            return PointLights;
        } else if constexpr (std::is_same_v<T, SpotLightUniformObjects>) {
            return SpotLights;
        } else {
            return nullptr;
        }
    }

    DirectionalLightUniformObjects DirLight{};

    static constexpr int MAX_POINT_LIGHTS = 8;
    PointLightUniformObjects PointLights[MAX_POINT_LIGHTS]{};
    GLint UniPointLightCount = -1;

    static constexpr int MAX_SPOT_LIGHTS = 8;
    SpotLightUniformObjects SpotLights[MAX_SPOT_LIGHTS]{};
    GLint UniSpotLightCount = -1;

    OmniShadowMapUniformObjects OmniShadowMap[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS]{};

private:
    bool ResetProgram();
    bool AddShaderFromSource(const std::string& src, GLenum type);

    GLint get(const char* n);
    std::string ReadFile(const std::filesystem::path& path);

    bool LinkAndReflect();
    void ReflectUniforms();

private:
    GLuint program_ = 0;

    // common uniforms
    GLint uniModel_ = -1, uniView_ = -1, uniProj_ = -1;
    GLint uniEyePos_ = -1, uniSpecularIntensity_ = -1, uniShininess_ = -1;
    GLint uniTexture_ = -1;
    GLint uniDirectionalLightTransform_ = -1, uniDirectionalShadowMap_ = -1;

    // omni / point-light shadow (cubemap) uniforms
    GLint uniOmniLightPos_ = -1, uniOmniFarPlane_ = -1;
    GLint uniLightMatrices_[6] = { -1,-1,-1,-1,-1,-1 };
};
