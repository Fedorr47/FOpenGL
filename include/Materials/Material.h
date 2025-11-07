
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Material {
public:
    Material(float specularIntensity, float shininess)
    : specularIntensity_(specularIntensity), shininess_(shininess) {}

    void Use(GLint specLoc, GLint shinLoc) const;
    void UseDSA(GLuint program, GLint specLoc, GLint shinLoc) const;

private:
    float specularIntensity_;
    float shininess_;
};
