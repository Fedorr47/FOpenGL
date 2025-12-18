#pragma once

// General headers
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <Model/Model.h>

#include "Light/Light.h"

// forward declaration
class GLWindow;
class Shader;
class Camera;
class Mesh;
class Material;
class Texture;
class PointLight;
class SpotLight;
class GameClock;
class DirectionalLight;
class Model;

namespace Assimp
{
    class Importer;
}

class GLApplication {
public:
    GLApplication(int width, int height, const char* title);
    ~GLApplication();

    //---------- Lights Start ----------//
    void AddPointLights();
    void AddSpotLights();
    void CreateShaders();
    void CreateTextures();
    void CreateDirectionalLight();
    //---------- Lights End  ----------//
    void Run();
    void SetGameInputMode(bool enabled);

    void AddModels();

private:
    void CreateScene();
    void RenderScene(Shader& sh, bool depthOnly);
    void DrawUI();
    void DrawPyramids(glm::mat4& model);
    void DrawModels(glm::mat4& model, Shader& sh, bool depthOnly);
    void DirectionalShadowMapPass(std::shared_ptr<DirectionalLight> light);
    template <class T>
    void OmniShadowMapPass(std::shared_ptr<T> light);
    void RenderPass();

private:
    std::unique_ptr<GLWindow> window_;
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Shader> directionalShadowshader_;
    std::unique_ptr<Shader> omniShadowshader_;

    std::shared_ptr<Camera> camera_;

    std::shared_ptr<DirectionalLight> dirLight_;
    std::vector<std::shared_ptr<PointLight>> pointLights_;
    std::vector<std::shared_ptr<SpotLight>> spotLights_;

    std::vector<std::unique_ptr<Mesh>> meshes_;
    std::vector<std::shared_ptr<Material>> materials_;
    std::vector<std::unique_ptr<Texture>> textures_;

    std::vector<std::shared_ptr<Model>> models_;

     std::unique_ptr<Assimp::Importer> assimpImporter;

    std::unique_ptr<GameClock> clock_;
    bool gameMode_{true};

    glm::mat4 projection_{};
};
