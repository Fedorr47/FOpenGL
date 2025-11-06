
#include "Rendering/Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() {}
Shader::~Shader() { Clear(); }

bool Shader::CreateFromSource(const std::string& vs, const std::string& fs)
{
    program_ = glCreateProgram();
    if (!program_) { std::cerr << "glCreateProgram failed\n"; return false; }

    if (!AddShader(program_, vs, GL_VERTEX_SHADER)) return false;
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
        char log[2048]; GLsizei n=0;
        glGetShaderInfoLog(sh, 2048, &n, log);
        std::cerr << "Compile error (" << type << "):\n" << log << "\n";
        glDeleteShader(sh);
        return false;
    }
    glAttachShader(program, sh);
    glDeleteShader(sh); // safe after attach
    return true;
}

bool Shader::LinkAndReflect()
{
    glLinkProgram(program_);
    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei n=0;
        glGetProgramInfoLog(program_, 2048, &n, log);
        std::cerr << "Link error:\n" << log << "\n";
        glDeleteProgram(program_); program_=0;
        return false;
    }
    glValidateProgram(program_);
    glGetProgramiv(program_, GL_VALIDATE_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei n=0;
        glGetProgramInfoLog(program_, 2048, &n, log);
        std::cerr << "Validate error:\n" << log << "\n";
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
    
    return true;
}

void Shader::Use() const { glUseProgram(program_); }
void Shader::Clear()
{
    if (program_) {
        glDeleteProgram(program_);
        program_=0;
    }
}
