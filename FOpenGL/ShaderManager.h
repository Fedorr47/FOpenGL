#pragma once

#include <GL/glew.h>

class ShaderManager
{
public:
    ShaderManager() {}

    // Objects
    void CreateTriangle();

    bool CompileShaders();

    void UseShaderProgram();

    GLuint shaderProgram;
    GLuint VAO;
    GLuint VBO;
    
private:
    bool AddShader(GLuint ShaderProgram, const char* ShaderSource, GLenum ShaderType);
};

