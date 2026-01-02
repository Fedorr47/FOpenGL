#pragma once

#include <iostream>

#include "Camera/Camera.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"

#include "Rendering/Shader.h"

template <typename LightType, typename LightUniformType, int MAX_LIGHTS>
struct LightsApplier
{
    static void ApplyLights(std::span<std::shared_ptr<LightType>> lights,
                            Shader& shader,
                            unsigned baseTextureUnit,
                            unsigned shadowIndexOffset)
    {
        auto uniforms = shader.GetUniformArray<LightUniformType>();
        if (!uniforms)
        {
            return;
        }

        const int count = std::min<int>((int)lights.size(), MAX_LIGHTS);

        for (int i = 0; i < count; ++i)
        {
            auto& Light = lights[i];
            if (!Light)
            {
                continue;
            }
            
            Light->UseLight(uniforms[i]);
            
            const unsigned slot = baseTextureUnit + static_cast<unsigned>(i);
            Light->GetLightProperties().shadowMapPtr->Read(GL_TEXTURE0 + slot);
            
            const unsigned idx = shadowIndexOffset + static_cast<unsigned>(i);

            glUniform1i(shader.OmniShadowMap[idx].ShadowMap, static_cast<GLint>(slot));
            glUniform1f(shader.OmniShadowMap[idx].farPlane, Light->GetLightProperties().farPlane);
        }
    }
};
