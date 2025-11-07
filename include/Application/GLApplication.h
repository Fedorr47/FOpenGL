#pragma once
#include <memory>
#include <vector>
#include <glm/mat4x4.hpp>

#include "Application/Time.h"
#include "Light/DirectionalLight.h"

class GLWindow;
class Shader;
class Mesh;
class Texture;
class Material;
class Camera;
class PointLight;

class GLApplication {
public:
    GLApplication(int width, int height, const char* title);
    ~GLApplication();

    void Run();

private:
    void CreateScene();
    void RenderFrame();
    void DrawUI();
    void OnResize(int w, int h);

private:
    std::unique_ptr<GLWindow>                 window_;
    std::unique_ptr<Shader>                   shader_;
    std::vector<std::unique_ptr<Mesh>>        meshes_;
    std::vector<std::unique_ptr<Texture>>     textures_;
    std::vector<std::shared_ptr<Material>>    materials_;
    std::shared_ptr<Camera>                   camera_;

    DirectionalLight                          dirLight_;
    std::vector<std::shared_ptr<PointLight>>  pointLights_;

    glm::mat4                                 projection_{1.0f};
    GameClock                                 clock_;
    bool                                      gameMode_{false};
};
