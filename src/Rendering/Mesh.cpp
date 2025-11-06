
#include "Rendering/Mesh.h"

Mesh::~Mesh(){ Clear(); }

void Mesh::Create(const std::vector<float>& vertices,
                  const std::vector<unsigned int>& indices,
                  unsigned int strideFloats)
{
    indexCount_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &VAO_);
    glBindVertexArray(VAO_);

    glGenBuffers(1, &IBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &VBO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // layout: pos(3) uv(2) normal(3) -> strideFloats expected 8
    GLsizei stride = strideFloats * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::Draw() const
{
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::Clear()
{
    if (IBO_) { glDeleteBuffers(1, &IBO_); IBO_=0; }
    if (VBO_) { glDeleteBuffers(1, &VBO_); VBO_=0; }
    if (VAO_) { glDeleteVertexArrays(1, &VAO_); VAO_=0; }
}
