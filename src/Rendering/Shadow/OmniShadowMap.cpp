#include "Rendering/Shadow/OmniShadowMap.h"

bool OmniShadowMap::Initialize(int width, int height)
{
    _shadowWidth = width;
    _shadowHeight = height;

    glGenFramebuffers(1, &_FBO);

    glGenTextures(1, &_shadowMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _shadowMap);

    for (GLuint i = 0; i < 6; i++)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_DEPTH_COMPONENT,
            _shadowWidth,
            _shadowHeight,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _shadowMap, 0);
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return status == GL_FRAMEBUFFER_COMPLETE;
}

void OmniShadowMap::Write()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
}

void OmniShadowMap::Read(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _shadowMap);
}