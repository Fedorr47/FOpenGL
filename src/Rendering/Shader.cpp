#include "Rendering/Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader() { Clear(); }

bool Shader::CreateFromSource(const std::string& vs, const std::string& fs)
{
    program_ = glCreateProgram();
    if (!program_) { std::cerr << "glCreateProgram failed\n"; return false; }

    if (!AddShader(program_, vs, GL_VERTEX_SHADER))   return false;
    if (!AddShader(program_, fs, GL_FRAGMENT_SHADER)) return false;
    return LinkAndReflect();
}

bool Shader::CreateFromFiles(const std::filesystem::path& vs, const std::filesystem::path& fs)
{
    return CreateFromSource(ReadFile(vs), ReadFile(fs));
}

std::string Shader::ReadFile(const std::filesystem::path& path)
{
    std::filesystem::path p = path.is_absolute() ? path : (std::filesystem::current_path() / path);
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open file: " + p.string());
    std::ostringstream ss; ss << f.rdbuf();
    return ss.str();
}

bool Shader::AddShader(GLuint program, const std::string& src, GLenum type)
{
    GLuint sh = glCreateShader(type);
    if (!sh) return false;
    const GLchar* ptr = src.data();
    GLint len = static_cast<GLint>(src.size());
    glShaderSource(sh, 1, &ptr, &len);
    glCompileShader(sh);

    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096]; GLsizei n=0;
        glGetShaderInfoLog(sh, sizeof(log), &n, log);
        std::cerr << "Compile error (" << (type==GL_VERTEX_SHADER?"VS":"FS") << "):\n" << log << "\n";
        glDeleteShader(sh);
        return false;
    }
    glAttachShader(program, sh);
    glDeleteShader(sh);
    return true;
}

bool Shader::LinkAndReflect()
{
    glLinkProgram(program_);
    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096]; GLsizei n=0;
        glGetProgramInfoLog(program_, sizeof(log), &n, log);
        std::cerr << "Link error:\n" << log << "\n";
        glDeleteProgram(program_); program_=0;
        return false;
    }

#ifndef NDEBUG
    glValidateProgram(program_);
    glGetProgramiv(program_, GL_VALIDATE_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei n=0;
        glGetProgramInfoLog(program_, sizeof(log), &n, log);
        std::cerr << "Validate warning:\n" << log << "\n";
    }
#endif

    auto get = [&](const char* n)->GLint {
        GLint loc = glGetUniformLocation(program_, n);
        return loc;
    };

    // ——— legacy-uniforms  ———
    uniModel_  = get("model");
    uniView_   = get("view");
    uniProj_   = get("projection");
    uniEyePos_ = get("eyePosition");
    uniSpecularIntensity_ = get("material.specularIntensity");
    uniShininess_         = get("material.shininess");

    DirLight.Colour           = get("directionalLight.base.colour");
    DirLight.AmbientIntensity = get("directionalLight.base.ambientIntensity");
    DirLight.DiffuseIntensity = get("directionalLight.base.diffuseIntensity");
    DirLight.Direction        = get("directionalLight.direction");

    UniPointLightCount = get("pointLightCount");
    for (int i=0; i<MAX_POINT_LIGHTS; ++i) {
        std::string b = "pointLights[" + std::to_string(i) + "].";
        PointLights[i].Colour           = get((b + "base.colour").c_str());
        PointLights[i].AmbientIntensity = get((b + "base.ambientIntensity").c_str());
        PointLights[i].DiffuseIntensity = get((b + "base.diffuseIntensity").c_str());
        PointLights[i].Position         = get((b + "position").c_str());
        PointLights[i].Constant         = get((b + "constant").c_str());
        PointLights[i].Linear           = get((b + "linear").c_str());
        PointLights[i].Exponent         = get((b + "exponent").c_str());
    }

    /*
    // ——— modern
    if (GLint loc = glGetUniformLocation(program_, "theTexture"); loc >= 0)
        glProgramUniform1i(program_, loc, 0); // theTexture -> unit 0
    */

    return true;
}

void Shader::BindUniformBlock(const char* blockName, GLuint binding) const
{
    GLuint idx = glGetUniformBlockIndex(program_, blockName);
    if (idx != GL_INVALID_INDEX) {
        glUniformBlockBinding(program_, idx, binding);
    } else {
#ifndef NDEBUG
        std::cerr << "Warn: uniform block not found: " << blockName << "\n";
#endif
    }
}

void Shader::BindStorageBlock(const char* blockName, GLuint binding) const
{
    // GL 4.3+: Shader storage blocks
    GLuint idx = glGetProgramResourceIndex(program_, GL_SHADER_STORAGE_BLOCK, blockName);
    if (idx != GL_INVALID_INDEX) {
        glShaderStorageBlockBinding(program_, idx, binding);
    } else {
#ifndef NDEBUG
        std::cerr << "Warn: storage block not found: " << blockName << "\n";
#endif
    }
}

void Shader::BindSamplerUnit(const char* samplerName, GLint unit) const
{
    if (GLint loc = glGetUniformLocation(program_, samplerName); loc >= 0) {
        glProgramUniform1i(program_, loc, unit);
    } else {
#ifndef NDEBUG
        std::cerr << "Warn: sampler uniform not found: " << samplerName << "\n";
#endif
    }
}

GLint Shader::GetUniformLocation(const char* name) const
{
    return glGetUniformLocation(program_, name);
}

void Shader::Clear()
{
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

DirectionalLightUniformObjects Shader::GetDirectionalLightUniforms() const {
    DirectionalLightUniformObjects u{};
    u.AmbientColourLocation   = static_cast<GLuint>(DirLight.Colour);
    u.AmbientIntensityLocation= static_cast<GLuint>(DirLight.AmbientIntensity);
    u.DiffuseIntensityLocation= static_cast<GLuint>(DirLight.DiffuseIntensity);
    u.DirectionLocation       = static_cast<GLuint>(DirLight.Direction);
    return u;
}

PointLightUniformObjects Shader::GetPointLightUniforms(int i) const {
    PointLightUniformObjects u{};
    u.AmbientColourLocation        = static_cast<GLuint>(PointLights[i].Colour);
    u.AmbientIntensityLocation     = static_cast<GLuint>(PointLights[i].AmbientIntensity);
    u.DiffuseIntensityLocation     = static_cast<GLuint>(PointLights[i].DiffuseIntensity);
    u.PositionLocation             = static_cast<GLuint>(PointLights[i].Position);
    u.ConstantAttenuationLocation  = static_cast<GLuint>(PointLights[i].Constant);
    u.LinearAttenuationLocation    = static_cast<GLuint>(PointLights[i].Linear);
    u.AttenuationExponentLocation  = static_cast<GLuint>(PointLights[i].Exponent);
    return u;
}
