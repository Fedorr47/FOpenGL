
#include "Rendering/Texture.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

Texture::Texture(const std::string& file): file_(file) {}
Texture::~Texture() { Clear(); }

bool Texture::Load()
{
    unsigned char* data = stbi_load(file_.c_str(), &width_, &height_, &bpp_, 4);
    if (!data) return false;

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return true;
}

void Texture::Use(int unit) const {
    glBindTextureUnit(unit, tex_);
}

void Texture::Clear()
{
    if (tex_) { glDeleteTextures(1, &tex_); tex_=0; }
}
