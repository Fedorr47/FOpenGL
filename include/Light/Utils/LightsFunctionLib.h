#pragma once

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
        if (!uniforms) return;

        const int count = std::min<int>((int)lights.size(), MAX_LIGHTS);

        for (int i = 0; i < count; ++i)
        {
            auto& L = lights[i];
            if (!L) continue;
            
            L->UseLight(uniforms[i]);
            
            const unsigned slot = baseTextureUnit + (unsigned)i;
            L->GetLightProperties().shadowMapPtr->Read(GL_TEXTURE0 + slot);
            
            const unsigned idx = shadowIndexOffset + (unsigned)i;

            glUniform1i(shader.OmniShadowMap[idx].ShadowMap, (GLint)slot);
            glUniform1f(shader.OmniShadowMap[idx].farPlane, L->GetLightProperties().farPlane);
        }
    }
};
