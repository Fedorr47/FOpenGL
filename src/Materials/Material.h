
#pragma once
#include <GL/glew.h>

class Material {
public:
    Material(float specularIntensity, float shininess)
    : specularIntensity_(specularIntensity), shininess_(shininess) {}

    void Use(GLint specularIntensityLoc, GLint shininessLoc) const {
        glUniform1f(specularIntensityLoc, specularIntensity_);
        glUniform1f(shininessLoc, shininess_);
    }

private:
    float specularIntensity_;
    float shininess_;
};
