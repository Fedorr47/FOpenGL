#pragma once

#include <GL/glew.h>

class ShadowMap
{
public:
    ShadowMap();
    virtual ~ShadowMap();
    ShadowMap(const ShadowMap&){};
    ShadowMap& operator=(const ShadowMap&){};
    ShadowMap(ShadowMap&&){};
    ShadowMap& operator=(ShadowMap&&){};

    virtual bool Initialize(int width, int height);
    virtual void Write();
    virtual void Read(GLenum textureUnit);

    GLuint GetShadowHeight() {return _shadowHeight; }
    GLuint GetShadowWidth() {return _shadowWidth; }
    
private:
    GLuint _FBO;
    GLuint _shadowMap;
    GLuint _shadowWidth, _shadowHeight;
};
