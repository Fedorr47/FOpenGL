
#pragma once
#include <string>
#include <GL/glew.h>

class Texture {
public:
    Texture() = default;
    explicit Texture(const std::string& file);
    ~Texture();

    bool Load();
    void Use(GLenum unit = GL_TEXTURE0) const;
    void Clear();

private:
    std::string file_;
    GLuint tex_ = 0;
    int width_ = 0, height_ = 0, bpp_ = 0;
};
