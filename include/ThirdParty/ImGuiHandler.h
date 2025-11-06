
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

inline void DrawPointLightsGui(std::vector<PointLight>& pls) {
    ImGui::Begin("Point Lights");
    for (size_t i=0;i<pls.size();++i) {
        if (ImGui::TreeNode((void*)i, "PointLight %zu", i)) {
            auto& p = pls[i].GetProperties();
            ImGui::ColorEdit3("Colour", &p.Colour.x);
            ImGui::DragFloat3("Position", &p.Position.x, 0.05f);
            ImGui::SliderFloat("Ambient", &p.AmbientIntensity, 0.0f, 2.0f);
            ImGui::SliderFloat("Diffuse", &p.DiffuseIntensity, 0.0f, 5.0f);
            ImGui::InputFloat("Constant", &p.Constant);
            ImGui::InputFloat("Linear", &p.Linear);
            ImGui::InputFloat("Quadratic", &p.Exponent);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

} // namespace ImGuiHandler
#endif
