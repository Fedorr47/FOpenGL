#include "Application/GLApplication.h"

// General headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils/RenderUtils.h"
#include "Light/Utils/LightsFunctionLib.h"

// Dependency headers
#include "Window/GLWindow.h"
#include "Rendering/Shader.h"
#include "Texture//Texture.h"
#include "Model//Mesh.h"
#include "Camera/Camera.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Materials/Material.h"
#include "Application/Time.h"
#include "Model/Model.h"
// Assimp
#include "assimp/Importer.hpp"

#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ThirdParty/ImGuiHandler.h"
#endif

GLApplication::GLApplication(int width, int height, const char* title)
{
    window_ = std::make_unique<GLWindow>(width, height, title);
    clock_ = std::make_unique<GameClock>();
    clock_->resetTime();

    assimpImporter = std::make_unique<Assimp::Importer>();
    //clock_.setFixedStep(1.0/60.0);
    //clock_.setMaxFrameClamp(0.1);
    //clock_.setMaxCatchUp(0.25);
}

GLApplication::~GLApplication()
{
#ifdef USE_IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

void GLApplication::AddPointLights()
{
    PointLightProperties plp;

    // TODO: Point lights needed to be optimized 
    plp.Colour = {1,0,0};
    plp.Position = {-3,1,-2.5};
    pointLights_.push_back(std::make_shared<PointLight>(plp));
    
    plp.Colour = {0,0,1};
    plp.Position = { 3,1,-2.5};
    pointLights_.push_back(std::make_shared<PointLight>(plp));
}

void GLApplication::AddSpotLights()
{
    SpotLightProperties slp{};
    slp.Colour= {0,1,1};
    slp.Edge = 20.0f;
    slp.Position  = {0.f, 5.f, -2.5f};
    slp.Direction = {0.f, -1.f, 0.f}; 

    spotLights_.push_back(std::make_shared<SpotLight>(slp));
    //slp.Colour = {0,0,1}; slp.Position = {-4,5,-3};
    //spotLights_.push_back(std::make_shared<SpotLight>(slp));
}

void GLApplication::CreateDirectionalLight()
{
    // Lights
    DirectionalLightProperties dlp;
    dlp.Colour = {1,1,1};
    dlp.AmbientIntensity = 0.1f;
    dlp.DiffuseIntensity = 0.6f;
    dlp.Direction = { 0.0f, -7.0f, -5.0f };
    dlp.shadowMapPtr = std::make_shared<ShadowMap>();
    dlp.shadowMapPtr->Initialize(2048, 2048);
    
    dirLight_ = std::make_shared<DirectionalLight>(DirectionalLight(dlp));
}

void GLApplication::AddModels()
{
    models_.push_back(std::make_shared<Model>());
    models_.back()->LoadModel("../assets/models/Intergalactic_Spaceship-(Wavefront).obj");
    models_.back()->SetPosition({-2,2,0.0f});
}

void GLApplication::CreateScene()
{
    // Camera
    camera_ = std::make_shared<Camera>(
        glm::vec3(0.0f, 1.0f, 5.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, 0.0f, 5.0f, 0.1f);

    projection_ = glm::perspective(glm::radians(45.0f),
        static_cast<float>(window_->GetBufferWidth())/static_cast<float>(window_->GetBufferHeight()),
        0.1f, 100.0f);

    // Shader
    shader_ = std::make_unique<Shader>();
    if (!shader_->CreateFromFiles("../shaders/basic.vert", "../shaders/basic.frag")) {
        throw std::exception("Failed to create shader.\n");
    }

    directionalShadowshader_ = std::make_unique<Shader>();
    if (!directionalShadowshader_->CreateFromFiles("../shaders/directional_shadow_map.vert",
        "../shaders/directional_shadow_map.frag")) {
        throw std::exception("Failed to create shadow shader.\n");
    }

    // Lights
    CreateDirectionalLight();
    AddPointLights();
    AddSpotLights();
    
    // Geometry: a simple floor quad and a pyramid
    std::vector<float> pyramid = {
        // pos             // uv     // normal (placeholder; not used in VS, we compute per-vertex usually)
        -1.f,-1.f,-0.6f,   0,0,     0,0,0,
         0.f,-1.f, 1.0f,   0.5,0,   0,0,0,
         1.f,-1.f,-0.6f,   1,0,     0,0,0,
         0.f, 1.f, 0.0f,   0.5,1,   0,0,0
    };
    std::vector<unsigned> pidx = {0,3,1, 1,3,2, 2,3,0, 0,1,2};
    ComputeAverageNormals(pyramid, pidx, 8, 5);

    auto m1 = std::make_unique<Mesh>();
    m1->Create(pyramid, pidx, 8);
    meshes_.push_back(std::move(m1));

    std::vector<float> floor = {
        -10,0,-10,  0,0,    0,1,0,
         10,0,-10, 10,0,    0,1,0,
        -10,0, 10,  0,10,   0,1,0,
         10,0, 10, 10,10,   0,1,0,
    };
    std::vector<unsigned> fidx = {0,2,1, 1,2,3};
    auto m2 = std::make_unique<Mesh>();

    ComputeAverageNormals(floor, fidx, 8, 5);
    m2->Create(floor, fidx, 8);
    meshes_.push_back(std::move(m2));

    // Textures
    auto tex1 = std::make_unique<Texture>("assets/textures/brick.png");
    tex1->LoadA();
    textures_.push_back(std::move(tex1));
    
    auto tex2 = std::make_unique<Texture>("assets/textures/dirt.png");
    tex2->LoadA();
    textures_.push_back(std::move(tex2));
    
    auto tex3 = std::make_unique<Texture>("assets/textures/plain.png");
    tex3->LoadA();
    textures_.push_back(std::move(tex3));

    // Materials
    materials_.push_back(std::make_shared<Material>(1.0f,64.0f)); // shiny
    materials_.push_back(std::make_shared<Material>(0.3f,8.0f));  // dull

    AddModels();

#ifdef USE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_->GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
}

void GLApplication::DrawUI()
{
#ifdef USE_IMGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Info");
    ImGui::Text("FPS: %.1f", clock_->getState().fps);
    ImGui::Text("WASD/mouse; TAB toggles mouse capture");
    ImGui::End();

    ImGuiHandler::DrawDirectionalLightGui(*dirLight_);
    int CurrentLightPointsNum = Shader::MAX_POINT_LIGHTS;
    ImGuiHandler::DrawPointLightsGui(
        pointLights_,
        static_cast<int>(Shader::MAX_POINT_LIGHTS));
    ImGuiHandler::DrawModelManipulatorGui(models_);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void GLApplication::DrawPyramids(glm::mat4& model)
{
    model = glm::translate(model, {0,0,-2.5f});
    glUniformMatrix4fv(shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[0]->Use(GL_TEXTURE0);
    materials_[0]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[0]->Draw();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, {0,4,-2.5f});
    glUniformMatrix4fv(shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[1]->Use(GL_TEXTURE0);
    materials_[1]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[0]->Draw();
}

void GLApplication::DrawModels(glm::mat4& model, Shader& sh, bool depthOnly)
{
    for (std::shared_ptr<Model>& modelObj : models_)
    {
        model = glm::translate(model, modelObj->GetTranslation());
        model = glm::scale(model, modelObj->GetScale());

        glm::vec3 r = glm::radians(modelObj->GetRotation()); // rotDeg -> radians
        model = glm::rotate(model, r.x, glm::vec3(1,0,0));
        model = glm::rotate(model, r.y, glm::vec3(0,1,0));
        model = glm::rotate(model, r.z, glm::vec3(0,0,1));
        
        glUniformMatrix4fv(sh.GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
        if (!depthOnly) {
            materials_[1]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
            modelObj->Render();
        } else {
            modelObj->Render();  
        }
    }
}

void GLApplication::DirectionalShadowMapPass(std::shared_ptr<DirectionalLight> light)
{
    auto smap = light->GetProperties().shadowMapPtr;
    directionalShadowshader_->Use();
    
    glViewport(0, 0, smap->GetShadowWidth(), smap->GetShadowHeight());
    smap->Write();
    glClear(GL_DEPTH_BUFFER_BIT); 
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
    glCullFace(GL_FRONT);
    
    directionalShadowshader_->SetDirectionalLightTransform(light->CalculateLightTransform());
    
    RenderScene(*directionalShadowshader_, /*depthOnly=*/true);
    
    glCullFace(GL_BACK);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLApplication::RenderPass()
{
    shader_->Use();

    glUniformMatrix4fv(shader_->GetUniformProj(), 1, GL_FALSE, glm::value_ptr(projection_));
    glUniformMatrix4fv(shader_->GetUniformView(), 1, GL_FALSE, glm::value_ptr(camera_->GetViewMatrix()));
    glUniform3f(shader_->GetUniformEyePos(), camera_->GetPosition().x, camera_->GetPosition().y, camera_->GetPosition().z);

    glViewport(0, 0, window_->GetBufferWidth(), window_->GetBufferHeight());
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    dirLight_->UseLight(shader_->DirLight);
    glUniform1i(shader_->UniPointLightCount, (GLint)pointLights_.size());
    glUniform1i(shader_->UniSpotLightCount,  (GLint)spotLights_.size());
    shader_->SetDirectionalLightTransform(dirLight_->CalculateLightTransform());

    dirLight_->GetProperties().shadowMapPtr->Read(GL_TEXTURE1);
    shader_->SetTexture(0);                  // diffuse sampler -> unit 0
    shader_->SetDirectionalShadowMap(1);     // shadow sampler  -> unit 1

    LightsApplier<PointLight, PointLightUniformObjects, Shader::MAX_POINT_LIGHTS>::ApplyLights(pointLights_, *shader_);
    if (!spotLights_.empty())
        spotLights_[0]->SetFlash(camera_->GetPosition(), camera_->GetDirection());
    LightsApplier<SpotLight,  SpotLightUniformObjects,  Shader::MAX_SPOT_LIGHTS >::ApplyLights(spotLights_,  *shader_);
    
    RenderScene(*shader_, /*depthOnly=*/false);
}

void GLApplication::RenderScene(Shader& sh, bool depthOnly)
{
    // Floor
    glm::mat4 model(1.f);
    model = glm::translate(model, {0,-2,-2.5f});
    glUniformMatrix4fv(sh.GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthOnly) {
        textures_[1]->Use(GL_TEXTURE0);
        materials_[1]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    }
    meshes_[1]->Draw();

    DrawModels(model, sh, depthOnly);
}

void GLApplication::Run()
{
    CreateScene();
    clock_->resetTime();

    bool prevTab=false;
    while (!glfwWindowShouldClose(window_->GetWindow())) {
        glfwPollEvents();
        
        bool tab = window_->GetKeys()[GLFW_KEY_TAB];
        if (tab && !prevTab) {
            gameMode_ = !gameMode_;
        }
        SetGameInputMode(gameMode_);

        prevTab = tab;

        clock_->beginFrame();
        const TimeState& clockState = clock_->getState();

#ifdef USE_IMGUI
        auto& io = ImGui::GetIO();
        bool uiMouse = io.WantCaptureMouse;
        bool uiKey = io.WantCaptureKeyboard || io.WantTextInput;
#else
        bool uiMouse = false, uiKey=false;
#endif

        if (gameMode_ && !uiKey)
        {
            camera_->KeyControl(window_->GetKeys(), static_cast<float>(clockState.deltaSec));
        }
        if (gameMode_ && !uiMouse)
        {
            camera_->MouseControl(window_->GetDeltaX(), window_->GetDeltaY(), static_cast<float>(clockState.deltaSec));
        }
        window_->ResetDelta();

        if (window_->IsResized())
        {
            projection_ = glm::perspective(glm::radians(45.0f),
                static_cast<float>(window_->GetBufferWidth())/static_cast<float>(window_->GetBufferHeight()),
            0.1f, 100.0f);
        }

        DirectionalShadowMapPass(dirLight_);
        RenderPass();
        DrawUI();

        glfwSwapBuffers(window_->GetWindow());
        window_->SetResized(false);
    }
}

void GLApplication::SetGameInputMode(bool enabled)
{
    GLFWwindow* window = window_->GetWindow();

#ifdef USE_IMGUI
    auto& io = ImGui::GetIO();
    if (enabled) {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    } else {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        io.MouseDrawCursor = false;
    }
#endif

    if (enabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    } else {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
