#pragma once

#include <GL/glew.h>

class ShadowMap
{
public:
    ShadowMap();
    virtual ~ShadowMap();
    ShadowMap(const ShadowMap&): _FBO(0), _shadowMap(0), _shadowWidth(0), _shadowHeight(0)
    {}
    ShadowMap& operator=(const ShadowMap&) { return *this; }
    ShadowMap(ShadowMap&&) = default;
    ShadowMap& operator=(ShadowMap&&) = default;

    virtual bool Initialize(int width, int height);
    virtual void Write();
    virtual void Read(GLenum textureUnit);

    GLuint GetShadowHeight() const {return _shadowHeight; }
    GLuint GetShadowWidth() const {return _shadowWidth; }
    
protected:
    GLuint _FBO;
    GLuint _shadowMap;
    GLuint _shadowWidth, _shadowHeight;
};
