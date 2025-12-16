
#pragma once
#ifdef USE_IMGUI
#include <vector>
#include <imgui.h>
#include <algorithm>
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

template <typename T>
inline void DrawModelManipulatorGui(T&& models)
{
    if (!ImGui::Begin("Model Manipulator"))
    {
        ImGui::End();
        return;
    }
    
    if (models.empty())
    {
        ImGui::TextUnformatted("No models in scene.");
        ImGui::End();
        return;
    }

    static int active = 0;
    active = std::clamp(active, 0, (int)models.size() - 1);
    
    ImGui::SliderInt("Active model", &active, 0, (int)models.size() - 1);
    
    if (active < 0 || active >= (int)models.size() || !models[active])
    {
        ImGui::TextUnformatted("Active model is null.");
        ImGui::End();
        return;
    }

    auto& m = *models[active];
    
    glm::vec3 pos = m.GetTranslation();
    glm::vec3 rot = m.GetRotation(); // degrees
    glm::vec3 scl = m.GetScale();

    ImGui::Separator();
    ImGui::Text("Model #%d", active);

    bool changed = false;

    changed |= ImGui::DragFloat3("Position", &pos.x, 0.05f);
    changed |= ImGui::DragFloat3("Rotation (deg)", &rot.x, 0.5f);
    
    changed |= ImGui::DragFloat3("Scale", &scl.x, 0.01f, 0.001f, 1000.0f);
    scl.x = std::max(scl.x, 0.001f);
    scl.y = std::max(scl.y, 0.001f);
    scl.z = std::max(scl.z, 0.001f);

    if (changed)
    {
        m.SetPosition(pos);
        m.SetRotation(rot);
        m.SetScale(scl);
    }

    ImGui::Separator();
    
    if (ImGui::Button("Reset Pos")) { m.SetPosition({0.f, 0.f, 0.f}); }
    ImGui::SameLine();
    if (ImGui::Button("Reset Rot")) { m.SetRotation({0.f, 0.f, 0.f}); }
    ImGui::SameLine();
    if (ImGui::Button("Reset Scale")) { m.SetScale({1.f, 1.f, 1.f}); }

    ImGui::End();
}
    
inline void DrawPointLightsGui(std::vector<std::shared_ptr<PointLight>>& lights, int maxCount)
{
    if (!ImGui::Begin("Point Lights"))
    {
        ImGui::End();
        return;
    }
    
    int current = static_cast<int>(lights.size());
    current = std::clamp(current, 0, maxCount);
    
    int desired = current;
    ImGui::SliderInt("Active lights", &desired, 0, maxCount);
    
    if (desired != current) {
        if (desired < current) {
            lights.erase(lights.begin() + desired, lights.end());
        } else {
            while (static_cast<int>(lights.size()) < desired) {
                lights.push_back(std::make_shared<PointLight>(MakeDefaultPL()));
                lights.back() = std::make_shared<PointLight>(MakeDefaultPL());
            }
        }
        current = desired;
    }

    ImGui::Separator();
    
    for (int i = 0; i < current;) {
        auto& light = *lights[i];

        ImGui::PushID(i);
        const bool open = ImGui::TreeNodeEx("PointLight", ImGuiTreeNodeFlags_DefaultOpen, "PointLight %d", i);
        if (open)
        {
            auto props = light.GetProperties();

            float col[3] = { props.Colour.r, props.Colour.g, props.Colour.b };
            if (ImGui::ColorEdit3("Colour", col)) {
                props.Colour = { col[0], col[1], col[2] };
            }

            ImGui::SliderFloat("Ambient", &props.AmbientIntensity, 0.0f, 2.0f, "%.2f");
            ImGui::SliderFloat("Diffuse", &props.DiffuseIntensity, 0.0f, 5.0f, "%.2f");

            ImGui::DragFloat3("Position", &props.Position.x, 0.05f);
            
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

            ImGui::InputFloat("Constant",  &props.Constant,  0.01f,  0.1f,  "%.3f");
            ImGui::InputFloat("Linear",    &props.Linear,    0.001f, 0.01f, "%.4f");
            ImGui::InputFloat("Quadratic", &props.Exponent,  0.0001f,0.001f,"%.5f");

            props.Constant = std::max(props.Constant, 0.0f);
            props.Linear   = std::max(props.Linear,   0.0f);
            props.Exponent = std::max(props.Exponent, 0.0f);

            if (ImGui::Button("Reset")) {
                props = MakeDefaultPL();
            }
            ImGui::SameLine();
            bool deleted = false;
            if (ImGui::Button("Delete")) {
                lights.erase(lights.begin() + i);
                --current;
                deleted = true;
            }
            
            if (!deleted) {
                light.SetProperties(props);
                ++i;
            }

            ImGui::TreePop();
        }
        else
        {
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                lights.erase(lights.begin() + i);
                --current;
                ImGui::PopID();
                continue;
            }
            ++i;
        }
        ImGui::PopID();
        ImGui::Separator();
    }
    
    if (ImGui::Button("Add") && static_cast<int>(lights.size()) < maxCount) {
        lights.push_back(std::make_shared<PointLight>(MakeDefaultPL()));
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        lights.clear();
    }

    ImGui::End();
}

} // namespace ImGuiHandler
#endif
