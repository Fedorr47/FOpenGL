#pragma once
#include <string>
#include <filesystem>
#include <glad/glad.h>
#include "Light/LightProperties.h"

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool CreateFromSource(const std::string& vs, const std::string& fs);
    bool CreateFromFiles(const std::filesystem::path& vs, const std::filesystem::path& fs);

    void Use() const { GLint cur = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
        if ((GLuint)cur != program_) glUseProgram(program_); }
    GLuint Id() const { return program_; }
    void   Clear();

    // --- Modern helpers ---
    void BindUniformBlock(const char* blockName, GLuint binding) const;
    void BindStorageBlock(const char* blockName, GLuint binding) const;
    void BindSamplerUnit(const char* samplerName, GLint unit) const;

    // --- uniform getters ---
    GLint GetUniformModel()   const noexcept { return uniModel_; }
    GLint GetUniformView()    const noexcept { return uniView_;  }
    GLint GetUniformProj()    const noexcept { return uniProj_;  }
    GLint GetUniformEyePos()  const noexcept { return uniEyePos_; }

    GLint GetUniformSpecularIntensity() const noexcept { return uniSpecularIntensity_; }
    GLint GetUniformShininess()         const noexcept { return uniShininess_; }
    
    GLint GetUniformLocation(const char* name) const;

    DirectionalLightUniformObjects GetDirectionalLightUniforms() const;
    PointLightUniformObjects       GetPointLightUniforms(int index) const;
    
    enum { MAX_POINT_LIGHTS = 3 };
    struct {
        GLint Colour{-1}, AmbientIntensity{-1}, DiffuseIntensity{-1}, Direction{-1};
    } DirLight;
    struct {
        GLint Colour{-1}, AmbientIntensity{-1}, DiffuseIntensity{-1};
        GLint Position{-1}, Constant{-1}, Linear{-1}, Exponent{-1};
    } PointLights[MAX_POINT_LIGHTS];

    GLint UniPointLightCount{-1};
    GLint uniModel_{-1}, uniView_{-1}, uniProj_{-1}, uniEyePos_{-1};
    GLint uniSpecularIntensity_{-1}, uniShininess_{-1};

private:
    static std::string ReadFile(const std::filesystem::path& path);
    bool AddShader(GLuint program, const std::string& src, GLenum type);
    bool LinkAndReflect();

    GLuint program_{0};
};
