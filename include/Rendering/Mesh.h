#pragma once
#include <vector>
#include <glad/glad.h>

class Mesh {
public:
    ~Mesh() { Clear(); }
    
    void Create(const std::vector<float>& vertices,
                const std::vector<unsigned int>& indices,
                unsigned int strideFloats = 8);

    void Draw() const;

    void Clear();

private:
    GLuint VAO_ = 0;
    GLuint VBO_ = 0;
    GLuint IBO_ = 0;
    GLsizei indexCount_ = 0;
    GLsizei strideBytes_ = 0;
};