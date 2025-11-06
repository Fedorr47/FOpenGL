
#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Window/GLWindow.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "Rendering/Mesh.h"
#include "Camera/Camera.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Materials/Material.h"
#include "Application/Time.h"

class GLApplication {
public:
    GLApplication(int width, int height, const char* title);
    ~GLApplication();

    void Run();

private:
    void CreateScene();
    void RenderFrame();
    void DrawUI();

private:
    std::unique_ptr<GLWindow> window_;
    std::unique_ptr<Shader> shader_;

    std::shared_ptr<Camera> camera_;
    glm::mat4 projection_{};

    DirectionalLight dirLight_;
    std::vector<std::shared_ptr<PointLight>> pointLights_;

    std::vector<std::unique_ptr<Mesh>> meshes_;
    std::vector<std::shared_ptr<Material>> materials_;
    std::vector<std::unique_ptr<Texture>> textures_;

    GameClock clock_;
    bool gameMode_{true};
};
