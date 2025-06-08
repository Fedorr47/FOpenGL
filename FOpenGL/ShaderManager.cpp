#include "ShaderManager.h"
#include <iostream>

#include <GL/glew.h>

// Vertex Shader
static const char* vertexShaderSource = R"(
    #version 330 core

    layout (location = 0) in vec3 pos;

    void main()
    {
        gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
    }
)";

// Fragment Shader
static const char* fragmentShaderSource = R"(
    #version 330 core

    out vec4 colour;

    void main()
    {
        colour = vec4(0.5, 0.0, 0.0, 0.7);
    }
)";

void ShaderManager::CreateTriangle()
{
    GLfloat vertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool ShaderManager::CompileShaders()
{
    shaderProgram = glCreateProgram();

    if (!shaderProgram)
    {
        std::cerr << "Error creating shader program.\n";
        return false;
    }

    if (!AddShader(shaderProgram, vertexShaderSource, GL_VERTEX_SHADER)) return false;
    if (!AddShader(shaderProgram, fragmentShaderSource, GL_FRAGMENT_SHADER)) return false;

    GLint success = 0;
    GLchar infoLog[1024] = {0};

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Error linking shader program:\n" << infoLog << "\n";
        return false;
    }
    
    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Error validating shader program:\n" << infoLog << "\n";
        return false;
    }

    return true;
}

void ShaderManager::UseShaderProgram()
{
    glUseProgram(shaderProgram);

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
        
    glUseProgram(0);
}

bool ShaderManager::AddShader(GLuint ShaderProgram, const char* ShaderSource, GLenum ShaderType)
{
    GLuint shader = glCreateShader(ShaderType);
    if (shader == 0) {
        std::cerr << "Error creating shader type " << ShaderType << "\n";
        return false;
    }
        

    const GLchar* theCode[1];
    theCode[0] = ShaderSource;

    GLint codeLength[1];
    codeLength[0] = strlen(ShaderSource);
    
    glShaderSource(shader, 1, theCode, codeLength);
    glCompileShader(shader);

    GLint success = 0;
    GLchar infoLog[1024] = {0};

    glLinkProgram(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        std::cerr << "Error compiling shader type " << ShaderType << ":\n" << infoLog << "\n";
        return false;
    }

    glAttachShader(ShaderProgram, shader);
    return true;
}
