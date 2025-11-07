
#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture {
public:
    Texture() = default;
    explicit Texture(const std::string& file);
    ~Texture();

    bool Load();
    void Use(int unit) const;
    void Clear();

private:
    std::string file_;
    GLuint tex_ = 0;
    int width_ = 0, height_ = 0, bpp_ = 0;
};
