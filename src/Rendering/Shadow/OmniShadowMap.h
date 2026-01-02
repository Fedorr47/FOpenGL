#pragma once
#include "ShadowMap.h"

class OmniShadowMap : public ShadowMap
{    
public:
    OmniShadowMap() : ShadowMap() {}
    ~OmniShadowMap(){}
    OmniShadowMap(const OmniShadowMap& other){}
    OmniShadowMap(OmniShadowMap&& other){}
    OmniShadowMap& operator=(const OmniShadowMap& other){}
    OmniShadowMap& operator=(OmniShadowMap&& other){}

    bool Initialize(int width, int height) override;
    void Write() override;
    void Read(GLenum textureUnit) override;
};
