#include "Rendering/Shader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

Shader::Shader() {}
Shader::~Shader() { Clear(); }

bool Shader::ResetProgram()
{
    Clear();
    program_ = glCreateProgram();
    return program_ != 0;
}

std::string Shader::ReadFile(const std::filesystem::path& path)
{
    std::filesystem::path p = path.is_absolute() ? path : (std::filesystem::current_path() / path);
    std::ifstream shaderFile(p, std::ios::binary);
    if (!shaderFile)
        throw std::runtime_error("Failed to open file: " + p.string());

    std::ostringstream ss;
    ss << shaderFile.rdbuf();
    return ss.str();
}

bool Shader::AddShaderFromSource(const std::string& src, GLenum type)
{
    if (!program_)
        return false;

    GLuint sh = glCreateShader(type);
    if (!sh)
        return false;

    const GLchar* ptr = src.data();
    GLint len = static_cast<GLint>(src.size());
    glShaderSource(sh, 1, &ptr, &len);
    glCompileShader(sh);

    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048]; GLsizei n = 0;
        glGetShaderInfoLog(sh, 2048, &n, log);
        // std::cerr << "Compile error (" << type << "):\n" << log << "\n";
        glDeleteShader(sh);
        return false;
    }

    glAttachShader(program_, sh);
    glDeleteShader(sh); // safe after attach
    return true;
}

GLint Shader::get(const char* n)
{
    GLint loc = glGetUniformLocation(program_, n);
    // if (loc == -1) std::cerr << "Warn: uniform not found: " << n << "\n";
    return loc;
}

bool Shader::CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader)
{
    if (!ResetProgram())
        return false;

    if (!AddShaderFromSource(vertexShader, GL_VERTEX_SHADER))
        return false;
    if (!AddShaderFromSource(fragmentShader, GL_FRAGMENT_SHADER))
        return false;

    return LinkAndReflect();
}

bool Shader::CreateGeometryFromFile(const std::filesystem::path& geometryShader)
{
    if (!program_)
        return false;

    if (geometryShader.empty())
        return true;

    return AddShaderFromSource(ReadFile(geometryShader), GL_GEOMETRY_SHADER);
}

bool Shader::CreateFromFiles(
    const std::filesystem::path& vertexShader,
    const std::filesystem::path& fragmentShader,
    const std::filesystem::path& geometryShader)
{
    if (!ResetProgram())
        return false;

    if (!AddShaderFromSource(ReadFile(vertexShader), GL_VERTEX_SHADER))
        return false;
    if (!AddShaderFromSource(ReadFile(fragmentShader), GL_FRAGMENT_SHADER))
        return false;

    if (!geometryShader.empty())
    {
        if (!AddShaderFromSource(ReadFile(geometryShader), GL_GEOMETRY_SHADER))
            return false;
    }

    return LinkAndReflect();
}

bool Shader::LinkAndReflect()
{
    if (!program_)
        return false;

    glLinkProgram(program_);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048]; GLsizei n = 0;
        glGetProgramInfoLog(program_, 2048, &n, log);
        // std::cerr << "Link error:\n" << log << "\n";
        glDeleteProgram(program_);
        program_ = 0;
        return false;
    }
    
    ReflectUniforms();

    glValidateProgram(program_);
    glGetProgramiv(program_, GL_VALIDATE_STATUS, &ok);
    if (!ok)
    {
        char log[2048]; GLsizei n = 0;
        glGetProgramInfoLog(program_, 2048, &n, log);
        // std::cerr << "Validate error:\n" << log << "\n";
    }
    
    return true;
}

void Shader::ReflectUniforms()
{
    // common
    uniModel_ = get("model");
    uniView_  = get("view");
    uniProj_  = get("projection");
    uniEyePos_ = get("eyePosition");
    
    uniColour_ = get("colour");

    uniSpecularIntensity_ = get("material.specularIntensity");
    uniShininess_ = get("material.shininess");

    uniTexture_ = get("theTexture");
    uniDirectionalLightTransform_ = get("directionalLightTransform");
    uniDirectionalShadowMap_ = get("directionalShadowMap");

    // directional light
    DirLight.Colour           = get("directionalLight.base.colour");
    DirLight.AmbientIntensity = get("directionalLight.base.ambientIntensity");
    DirLight.DiffuseIntensity = get("directionalLight.base.diffuseIntensity");
    DirLight.Direction        = get("directionalLight.direction");

    // point lights
    UniPointLightCount = get("pointLightCount");
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        std::string b = "pointLights[" + std::to_string(i) + "].";
        PointLights[i].Colour           = get((b + "base.colour").c_str());
        PointLights[i].AmbientIntensity = get((b + "base.ambientIntensity").c_str());
        PointLights[i].DiffuseIntensity = get((b + "base.diffuseIntensity").c_str());
        PointLights[i].Position         = get((b + "position").c_str());
        PointLights[i].Constant         = get((b + "constant").c_str());
        PointLights[i].Linear           = get((b + "linear").c_str());
        PointLights[i].Exponent         = get((b + "exponent").c_str());
    }

    // spot lights
    UniSpotLightCount = get("spotLightCount");
    for (int i = 0; i < MAX_SPOT_LIGHTS; ++i) {
        std::string b = "spotLights[" + std::to_string(i) + "].";
        SpotLights[i].Colour           = get((b + "base.base.colour").c_str());
        SpotLights[i].AmbientIntensity = get((b + "base.base.ambientIntensity").c_str());
        SpotLights[i].DiffuseIntensity = get((b + "base.base.diffuseIntensity").c_str());

        SpotLights[i].Position         = get((b + "base.position").c_str());
        SpotLights[i].Constant         = get((b + "base.constant").c_str());
        SpotLights[i].Linear           = get((b + "base.linear").c_str());
        SpotLights[i].Exponent         = get((b + "base.exponent").c_str());

        SpotLights[i].Direction        = get((b + "direction").c_str());
        SpotLights[i].Edge             = get((b + "edge").c_str());
    }
    
    // omni / point light shadow map (cubemap)
    uniOmniLightPos_ = get("lightPos");
    uniOmniFarPlane_ = get("farPlane");
    for (int i = 0; i < 6; ++i)
    {
        std::string n = "lightMatrices[" + std::to_string(i) + "]";
        uniLightMatrices_[i] = get(n.c_str());
    }

    for (int i = 0; i < MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; ++i)
    {
        std::string b = "OmniShadowMaps[" + std::to_string(i) + "].";
        OmniShadowMap[i].ShadowMap = get((b + "shadowMap").c_str());
        OmniShadowMap[i].farPlane = get((b + "farPlane").c_str());
    }
}

void Shader::BindDefaultSamplers() const
{
    Use();

    // main material
    if (uniTexture_ != -1)
    {
        glUniform1i(uniTexture_, 0);
    }

    // directional shadow
    if (uniDirectionalShadowMap_ != -1)
    {
        glUniform1i(uniDirectionalShadowMap_, 1);
    }

    // omni shadow cube maps: unit 2.. (2+MAX-1)
    for (int i = 0; i < MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; ++i)
    {
        if (OmniShadowMap[i].ShadowMap != -1)
        {
            glUniform1i(OmniShadowMap[i].ShadowMap, 2 + i);
        }
    }
}

void Shader::Use() const
{
    glUseProgram(program_);
}

void Shader::SetTexture(GLuint textureUnit)
{
    if (uniTexture_ != -1)
        glUniform1i(uniTexture_, static_cast<GLint>(textureUnit));
}

void Shader::SetDirectionalShadowMap(GLuint textureUnit)
{
    if (uniDirectionalShadowMap_ != -1)
        glUniform1i(uniDirectionalShadowMap_, static_cast<GLint>(textureUnit));
}

void Shader::SetDirectionalLightTransform(const glm::mat4* lTransform)
{
    if (!lTransform) return;
    if (uniDirectionalLightTransform_ != -1)
        glUniformMatrix4fv(uniDirectionalLightTransform_, 1, GL_FALSE, glm::value_ptr(*lTransform));
}

void Shader::SetLightMatrices(const std::vector<glm::mat4>& lightMatrices)
{
    for (int i = 0; i < 6; ++i)
    {
        if (uniLightMatrices_[i] != -1)
            glUniformMatrix4fv(uniLightMatrices_[i], 1, GL_FALSE, glm::value_ptr(lightMatrices[i]));
    }
}

void Shader::Clear()
{
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}