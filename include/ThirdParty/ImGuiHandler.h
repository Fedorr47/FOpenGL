
#pragma once
#ifdef USE_IMGUI
#include <vector>
#include <imgui.h>
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"

namespace ImGuiHandler {

inline void DrawDirectionalLightGui(DirectionalLight& dl) {
    ImGui::Begin("Directional Light");
    auto& p = dl.GetProperties();
    ImGui::ColorEdit3("Colour", &p.Colour.x);
    ImGui::DragFloat3("Direction", &p.Direction.x, 0.01f, -1.0f, 1.0f);
    ImGui::SliderFloat("Ambient", &p.AmbientIntensity, 0.0f, 2.0f);
    ImGui::SliderFloat("Diffuse", &p.DiffuseIntensity, 0.0f, 5.0f);
    ImGui::End();
}

inline PointLightProperties MakeDefaultPL()
{
    PointLightProperties p{};
    p.Colour = {1.0f, 1.0f, 1.0f};
    p.AmbientIntensity = 0.1f;
    p.DiffuseIntensity = 1.0f;
    p.Position = {0.0f, 1.0f, 0.0f};
    p.Constant = 1.0f;
    p.Linear   = 0.09f;
    p.Exponent = 0.032f;
    return p;
}

struct AttenPreset { const char* name; float c,l,q; };
static constexpr AttenPreset kPresets[] = {
    {"None (no falloff)", 1.0f, 0.0f,   0.0f},
    {"Near (soft)",       1.0f, 0.22f,  0.20f},
    {"Medium",            1.0f, 0.14f,  0.07f},
    {"Far",               1.0f, 0.09f,  0.032f},
};
    
inline void DrawPointLightsGui(std::span<std::shared_ptr<PointLight>> lights,
                                      int   maxCount)
{
    if (!ImGui::Begin("Point Lights")) { ImGui::End(); return; }

    int activeCount = lights.size();
    activeCount = std::clamp(activeCount, 0, maxCount);
    int newCount = activeCount;
    ImGui::SliderInt("Active lights", &newCount, 0, maxCount);
    if (newCount != activeCount) {
        for (int i = activeCount; i < newCount; ++i) {
            if (!lights[i]) lights[i] = std::make_shared<PointLight>(MakeDefaultPL());
        }
        activeCount = newCount;
    }

    ImGui::Separator();

    for (int i = 0; i < activeCount; ++i) {
        if (!lights[i])
        {
            lights[i] = std::make_shared<PointLight>(MakeDefaultPL());
        }
        auto& light = *lights[i];

        if (ImGui::TreeNodeEx((void*)(intptr_t)i, ImGuiTreeNodeFlags_DefaultOpen,
                              "PointLight %d", i))
        {
            auto props = light.GetProperties();

            float col[3] = { props.Colour.r, props.Colour.g, props.Colour.b };
            if (ImGui::ColorEdit3("Colour", col))
                props.Colour = { col[0], col[1], col[2] };


            ImGui::SliderFloat("Ambient", &props.AmbientIntensity, 0.0f, 2.0f, "%.2f");
            ImGui::SliderFloat("Diffuse", &props.DiffuseIntensity, 0.0f, 5.0f, "%.2f");


            ImGui::DragFloat3("Position", &props.Position.x, 0.05f);
            
            int preset = -1;
            if (ImGui::BeginCombo("Attenuation preset", "Select…")) {
                for (int p = 0; p < (int)std::size(kPresets); ++p) {
                    if (ImGui::Selectable(kPresets[p].name)) {
                        props.Constant = kPresets[p].c;
                        props.Linear   = kPresets[p].l;
                        props.Exponent = kPresets[p].q;
                    }
                }
                ImGui::EndCombo();
            }
            (void)preset;

            ImGui::InputFloat("Constant", &props.Constant, 0.01f, 0.1f, "%.3f");
            ImGui::InputFloat("Linear",   &props.Linear,   0.001f, 0.01f, "%.4f");
            ImGui::InputFloat("Quadratic",&props.Exponent, 0.0001f,0.001f,"%.5f");
            
            props.Constant = std::max(props.Constant, 0.0f);
            props.Linear   = std::max(props.Linear,   0.0f);
            props.Exponent = std::max(props.Exponent, 0.0f);
            
            if (ImGui::Button("Reset")) {
                props = MakeDefaultPL();
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                if (i != activeCount - 1)
                    std::swap(lights[i], lights[activeCount - 1]);
                lights[activeCount - 1].reset();
                --activeCount;
                ImGui::TreePop();
                continue;
            }
            
            light.SetProperties(props);

            ImGui::TreePop();
        }
        ImGui::Separator();
    }

    ImGui::End();
}

} // namespace ImGuiHandler
#endif
