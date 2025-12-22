#pragma once
#include <string>
#include <GL/glew.h>

enum class TextureType : uint8_t
{
    RGB,
    RGBA
};

class Texture {
public:
    Texture() = default;
    explicit Texture(std::string file);
    ~Texture();

    bool Load();
    bool LoadA();
    void Use(GLenum unit = GL_TEXTURE0) const;
    void Clear();

private:

    bool LoadFromFile(TextureType Type);
    
    std::string file_;
    GLuint tex_ = 0;
    int width_ = 0, height_ = 0, bpp_ = 0;
};
