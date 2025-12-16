
#include "Rendering/Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.inl>

Shader::Shader() {}
Shader::~Shader() { Clear(); }

bool Shader::CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader)
{
    program_ = glCreateProgram();
    if (!program_)
    {
        //std::cerr << "glCreateProgram failed\n";
        return false;
    }

    if (!AddShader(program_, vertexShader, GL_VERTEX_SHADER))
    {
        return false;
    }
    if (!AddShader(program_, fragmentShader, GL_FRAGMENT_SHADER))
    {
        return false;
    }
    return LinkAndReflect();
}

bool Shader::CreateFromFiles(const std::filesystem::path& vertexShader, const std::filesystem::path& fragmentShader)
{
    return CreateFromSource(ReadFile(vertexShader), ReadFile(fragmentShader));
}

std::string Shader::ReadFile(const std::filesystem::path& path)
{
    std::filesystem::path p = path.is_absolute() ? path : (std::filesystem::current_path() / path);
    std::ifstream shaderFile(p, std::ios::binary);
    if (!shaderFile)
    {
        throw std::runtime_error("Failed to open file: " + p.string());
    }
    std::ostringstream ss;
    ss << shaderFile.rdbuf();
    return ss.str();
}

bool Shader::AddShader(GLuint program, const std::string& src, GLenum type)
{
    GLuint shaderProgramm = glCreateShader(type);
    if (!shaderProgramm)
    {
        return false;
    }
    const GLchar* ptr = src.data();
    GLint len = static_cast<GLint>(src.size());
    glShaderSource(shaderProgramm, 1, &ptr, &len);
    glCompileShader(shaderProgramm);
    GLint ok = GL_FALSE;
    glGetShaderiv(shaderProgramm, GL_COMPILE_STATUS, &ok);
    if (!ok)
        {
        char log[2048]; GLsizei n=0;
        glGetShaderInfoLog(shaderProgramm, 2048, &n, log);
        //std::cerr << "Compile error (" << type << "):\n" << log << "\n";
        glDeleteShader(shaderProgramm);
        return false;
    }
    glAttachShader(program, shaderProgramm);
    glDeleteShader(shaderProgramm); // safe after attach
    return true;
}

bool Shader::LinkAndReflect()
{
    glLinkProgram(program_);
    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048]; GLsizei n=0;
        glGetProgramInfoLog(program_, 2048, &n, log);
        //std::cerr << "Link error:\n" << log << "\n";
        glDeleteProgram(program_); program_=0;
        return false;
    }
    glValidateProgram(program_);
    glGetProgramiv(program_, GL_VALIDATE_STATUS, &ok);
    if (!ok)
    {
        char log[2048]; GLsizei n=0;
        glGetProgramInfoLog(program_, 2048, &n, log);
        //std::cerr << "Validate error:\n" << log << "\n";
    }

    auto get = [&](const char* n)->GLint {
        GLint loc = glGetUniformLocation(program_, n);
        if (loc == -1) {
            // std::cerr << "Warn: uniform not found: " << n << "\n";
        }
        return loc;
    };

    uniModel_ = get("model");
    uniView_  = get("view");
    uniProj_  = get("projection");
    uniEyePos_ = get("eyePosition");
    uniSpecularIntensity_ = get("material.specularIntensity");
    uniShininess_ = get("material.shininess");
    uniTexture_ = get("theTexture");
    uniDirectionalLightTransform_ = get("directionalLightTransform");
    uniDirectionalShadowMap_ = get("directionalShadowMap");

    DirLight.Colour           = get("directionalLight.base.colour");
    DirLight.AmbientIntensity = get("directionalLight.base.ambientIntensity");
    DirLight.DiffuseIntensity = get("directionalLight.base.diffuseIntensity");
    DirLight.Direction        = get("directionalLight.direction");

    UniPointLightCount = get("pointLightCount");
    for (int i=0;i<MAX_POINT_LIGHTS;++i) {
        std::string b = "pointLights[" + std::to_string(i) + "].";
        PointLights[i].Colour           = get((b + "base.colour").c_str());
        PointLights[i].AmbientIntensity = get((b + "base.ambientIntensity").c_str());
        PointLights[i].DiffuseIntensity = get((b + "base.diffuseIntensity").c_str());
        PointLights[i].Position         = get((b + "position").c_str());
        PointLights[i].Constant         = get((b + "constant").c_str());
        PointLights[i].Linear           = get((b + "linear").c_str());
        PointLights[i].Exponent         = get((b + "exponent").c_str());
    }

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
    
    return true;
}

void Shader::Use() const
{
    glUseProgram(program_);
}

void Shader::SetTexture(GLuint textureUnit)
{
    glUniform1i(uniTexture_, textureUnit);
}
void Shader::SetDirectionalShadowMap(GLuint textureUnit)
{
    glUniform1i(uniDirectionalShadowMap_, textureUnit);
}
void Shader::SetDirectionalLightTransform(std::unique_ptr<glm::mat4> lTransform)
{
    glUniformMatrix4fv(
        uniDirectionalLightTransform_, 1, GL_FALSE, glm::value_ptr(*lTransform));
}

void Shader::Clear()
{
    if (program_) {
        glDeleteProgram(program_);
        program_=0;
    }
}
