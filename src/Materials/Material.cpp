#include "Materials/Material.h"

void Material::Use(GLint specLoc, GLint shinLoc) const {
    if (specLoc != -1) glUniform1f(specLoc, specularIntensity_);
    if (shinLoc != -1) glUniform1f(shinLoc, shininess_);
}

void Material::UseDSA(GLuint program, GLint specLoc, GLint shinLoc) const {
    if (specLoc != -1) glProgramUniform1f(program, specLoc, specularIntensity_);
    if (shinLoc != -1) glProgramUniform1f(program, shinLoc, shininess_);
}