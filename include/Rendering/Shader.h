#pragma once
#include <string>
#include <filesystem>
#include <GL/glew.h>
#include <Light/Light.h>

#include "Light/LightProperties.h"

class Shader {
public:
    Shader();
    ~Shader();

    bool CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader);
    bool CreateFromFiles(const std::filesystem::path& vertexShader, const std::filesystem::path& fragmentShader);

    void Use() const;
    void SetTexture(GLuint textureUnit);
    void SetDirectionalShadowMap(GLuint textureUnit);
    void SetDirectionalLightTransform(std::unique_ptr<glm::mat4> lTransform);
    void SetUniformModel(GLint uniformModel) { uniModel_ = uniformModel; }
    void Clear();

    GLint GetUniformModel() const { return uniModel_; }
    GLint GetUniformView() const { return uniView_; }
    GLint GetUniformProj() const { return uniProj_; }
    GLint GetUniformEyePos() const { return uniEyePos_; }
    GLint GetUniformSpecularIntensity() const { return uniSpecularIntensity_; }
    GLint GetUniformShininess() const { return uniShininess_; }

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
    PointLightUniformObjects PointLights[MAX_POINT_LIGHTS];
    GLint UniPointLightCount = -1;

    static constexpr int MAX_SPOT_LIGHTS = 8;
    SpotLightUniformObjects SpotLights[MAX_SPOT_LIGHTS];
    GLint UniSpotLightCount = -1;

private:
    bool AddShader(GLuint program, const std::string& src, GLenum type);
    std::string ReadFile(const std::filesystem::path& path);
    bool LinkAndReflect();

private:
    GLuint program_ = 0;
    // common uniforms
    GLint uniModel_ = -1, uniView_ = -1, uniProj_ = -1;
    GLint uniEyePos_ = -1, uniSpecularIntensity_ = -1, uniShininess_ = -1;
    GLint uniTexture_ = -1;
    GLint uniDirectionalLightTransform_ = -1, uniDirectionalShadowMap_ = -1;
};
