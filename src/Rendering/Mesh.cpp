#include "Rendering/Mesh.h"
#include <cassert>

void Mesh::Create(const std::vector<float>& vertices,
                  const std::vector<unsigned int>& indices,
                  unsigned int strideFloats)
{
    Clear();

    indexCount_  = static_cast<GLsizei>(indices.size());
    strideBytes_ = static_cast<GLsizei>(strideFloats * sizeof(float));
    assert(strideFloats >= 8 && "expected interleaved layout: pos3 uv2 nrm3");
    
    glCreateVertexArrays(1, &VAO_);
    glCreateBuffers(1, &VBO_);
    glCreateBuffers(1, &IBO_);
    
    glNamedBufferStorage(VBO_, vertices.size() * sizeof(float), vertices.data(), 0);
    glNamedBufferStorage(IBO_, indices.size()  * sizeof(unsigned int), indices.data(), 0);
    
    const GLuint binding = 0;
    glVertexArrayVertexBuffer(VAO_, binding, VBO_, /*offset*/0, /*stride*/strideBytes_);
    glVertexArrayElementBuffer(VAO_, IBO_);
    
    glEnableVertexArrayAttrib(VAO_, 0);
    glVertexArrayAttribBinding(VAO_, 0, binding);
    glVertexArrayAttribFormat (VAO_, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(VAO_, 1);
    glVertexArrayAttribBinding(VAO_, 1, binding);
    glVertexArrayAttribFormat (VAO_, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

    glEnableVertexArrayAttrib(VAO_, 2);
    glVertexArrayAttribBinding(VAO_, 2, binding);
    glVertexArrayAttribFormat (VAO_, 2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float));
}

void Mesh::Draw() const
{
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, /*indices*/nullptr);
    glBindVertexArray(0);
}

void Mesh::Clear()
{
    if (IBO_) { glDeleteBuffers(1, &IBO_); IBO_ = 0; }
    if (VBO_) { glDeleteBuffers(1, &VBO_); VBO_ = 0; }
    if (VAO_) { glDeleteVertexArrays(1, &VAO_); VAO_ = 0; }
}
