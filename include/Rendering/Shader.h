
#pragma once
#include <string>
#include <filesystem>
#include <GL/glew.h>

struct DirectionalLightUniform {
    GLint colour = -1;
    GLint ambientIntensity = -1;
    GLint diffuseIntensity = -1;
    GLint direction = -1;
};

struct PointLightUniform {
    GLint colour = -1;
    GLint ambientIntensity = -1;
    GLint diffuseIntensity = -1;
    GLint position = -1;
    GLint constant = -1;
    GLint linear = -1;
    GLint exponent = -1;
};

class Shader {
public:
    Shader();
    ~Shader();

    bool CreateFromSource(const std::string& vs, const std::string& fs);
    bool CreateFromFiles(const std::filesystem::path& vs, const std::filesystem::path& fs);

    void Use() const;
    void Clear();

    GLint GetUniformModel() const { return uniModel_; }
    GLint GetUniformView() const { return uniView_; }
    GLint GetUniformProj() const { return uniProj_; }
    GLint GetUniformEyePos() const { return uniEyePos_; }
    GLint GetUniformSpecularIntensity() const { return uniSpecularIntensity_; }
    GLint GetUniformShininess() const { return uniShininess_; }

    DirectionalLightUniform DirLight{};
    static constexpr int MAX_POINT_LIGHTS = 8;
    PointLightUniform PointLights[MAX_POINT_LIGHTS];
    GLint UniPointLightCount = -1;

private:
    bool AddShader(GLuint program, const std::string& src, GLenum type);
    std::string ReadFile(const std::filesystem::path& path);
    bool LinkAndReflect();

private:
    GLuint program_ = 0;
    // common uniforms
    GLint uniModel_ = -1, uniView_ = -1, uniProj_ = -1;
    GLint uniEyePos_ = -1, uniSpecularIntensity_ = -1, uniShininess_ = -1;
};
