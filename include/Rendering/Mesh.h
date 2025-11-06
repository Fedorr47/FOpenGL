
#pragma once
#include <vector>
#include <GL/glew.h>

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    void Create(const std::vector<float>& vertices,
                const std::vector<unsigned int>& indices,
                unsigned int strideFloats);

    void Draw() const;
    void Clear();

private:
    GLuint VAO_=0, VBO_=0, IBO_=0;
    GLsizei indexCount_ = 0;
};
