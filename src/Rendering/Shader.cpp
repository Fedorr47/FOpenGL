
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

    DirLight.colour = get("directionalLight.base.colour");
    DirLight.ambientIntensity = get("directionalLight.base.ambientIntensity");
    DirLight.diffuseIntensity = get("directionalLight.base.diffuseIntensity");
    DirLight.direction = get("directionalLight.direction");

    UniPointLightCount = get("pointLightCount");
    for (int i=0;i<MAX_POINT_LIGHTS;++i) {
        std::string base = "pointLights[" + std::to_string(i) + "].";
        PointLights[i].colour           = get((base + "base.colour").c_str());
        PointLights[i].ambientIntensity = get((base + "base.ambientIntensity").c_str());
        PointLights[i].diffuseIntensity = get((base + "base.diffuseIntensity").c_str());
        PointLights[i].position         = get((base + "position").c_str());
        PointLights[i].constant         = get((base + "constant").c_str());
        PointLights[i].linear           = get((base + "linear").c_str());
        PointLights[i].exponent         = get((base + "exponent").c_str());

        /*
        char locBuff[100] = { '\0' };

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.colour", i);
        PointLights[i].colour= glGetUniformLocation(program_, locBuff);

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.ambientIntensity", i);
        PointLights[i].ambientIntensity = glGetUniformLocation(program_, locBuff);

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.diffuseIntensity", i);
        PointLights[i].diffuseIntensity = glGetUniformLocation(program_, locBuff);

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].position", i);
        PointLights[i].position = glGetUniformLocation(program_, locBuff);

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].constant", i);
        PointLights[i].constant = glGetUniformLocation(program_, locBuff);

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].linear", i);
        PointLights[i].linear = glGetUniformLocation(program_, locBuff);

        snprintf(locBuff, sizeof(locBuff), "pointLights[%d].exponent", i);
        PointLights[i].exponent = glGetUniformLocation(program_, locBuff);
        */
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
