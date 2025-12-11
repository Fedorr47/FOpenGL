#pragma once

#include "Camera/Camera.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"

#include "Rendering/Shader.h"

template <typename LightType, typename LightUniformType, int MAX_LIGHTS>
struct LightsApplier
{
    static void ApplyLights(std::span<std::shared_ptr<LightType>> Lights, Shader& shader)
    {
        {
            auto LightUniforms = shader.GetUniformArray<LightUniformType>();
            if (LightUniforms == nullptr)
            {
                return;
            }
            for (int i = 0; i<static_cast<int>(Lights.size()) && i < MAX_LIGHTS; ++i)
            {
                if (Lights[i] == nullptr)
                {
                    continue;
                }
                Lights[i]->UseLight(LightUniforms[i]);
            }
        }
    }
};