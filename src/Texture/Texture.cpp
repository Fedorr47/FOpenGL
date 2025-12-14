
#include "Texture//Texture.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

Texture::Texture(std::string file): file_(std::move(file))
{
}

Texture::~Texture() { Clear(); }

bool Texture::Load()
{
    return LoadFromFile(TextureType::RGB);
}

bool Texture::LoadA()
{
    return LoadFromFile(TextureType::RGBA);
}

void Texture::Use(GLenum unit) const
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, tex_);
}

void Texture::Clear()
{
    if (tex_) { glDeleteTextures(1, &tex_); tex_=0; }
}

bool Texture::LoadFromFile(TextureType Type)
{
    unsigned char* data = stbi_load(file_.c_str(), &width_, &height_, &bpp_, 4);
    if (!data)
    {
        return false;
    }

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    switch (Type)
    {
    case TextureType::RGB:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        break;
    case TextureType::RGBA:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        break;
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return true;
}
