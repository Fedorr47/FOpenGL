#include "Application/GLApplication.h"

#include <algorithm>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window/GLWindow.h"
#include "Rendering/Shader.h"
#include "Rendering/Mesh.h"
#include "Rendering/Texture.h"
#include "Camera/Camera.h"
#include "Materials/Material.h"
#include "Materials/MaterialFactory.h"
#include "Light/PointLight.h"
#include "Utils/RenderUtils.h"

#ifdef USE_IMGUI
  #include <imgui.h>
  #include <imgui_impl_glfw.h>
  #include <imgui_impl_opengl3.h>
  #include "ThirdParty/ImGuiHandler.h"
#endif

GLApplication::GLApplication(int width, int height, const char* title)
{
    window_ = std::make_unique<GLWindow>(width, height, title);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_FRAMEBUFFER_SRGB);

    clock_.reset();
    // clock_.setFixedStep(1.0/60.0);
    // clock_.setMaxFrameClamp(0.1);
    // clock_.setMaxCatchUp(0.25);
}

GLApplication::~GLApplication()
{
#ifdef USE_IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

void GLApplication::CreateScene()
{
    camera_ = std::make_shared<Camera>(
        glm::vec3(0.0f, 1.0f, 5.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, 0.0f, 5.0f, 0.1f);

    projection_ = glm::perspective(
        glm::radians(45.0f),
        float(window_->GetBufferWidth()) / std::max(1.0f, float(window_->GetBufferHeight())),
        0.1f, 100.0f);
    
    shader_ = std::make_unique<Shader>();
    if (!shader_->CreateFromFiles("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "Failed to create shader.\n";
    }

    window_->onResize = [this](int w, int h){ OnResize(w,h); };window_->onResize = [this](int w, int h){ OnResize(w,h); };

    shader_->BindSamplerUnit("theTexture", 0);
    
    {
        DirectionalLightProperties dlp{};
        dlp.AmbientColour           = {1.0f, 1.0f, 1.0f};
        dlp.AmbientIntensity = 0.25f;
        dlp.DiffuseIntensity = 0.8f;
        dlp.Direction        = {-0.5f, -1.0f, -0.3f};
        dirLight_ = DirectionalLight(dlp);

        PointLightProperties plp{};
        plp.AmbientColour   = {1,0,0}; plp.AmbientIntensity=0.5f; plp.DiffuseIntensity=1.0f;
        plp.Position = {-3, 1, 0}; plp.Constant=0.3f; plp.Linear=0.1f; plp.Exponent=0.1f;
        pointLights_.push_back(std::make_shared<PointLight>(plp));

        plp.AmbientColour   = {0,0,1};
        plp.Position = { 3, 1, 0};
        pointLights_.push_back(std::make_shared<PointLight>(plp));
    }

    {
        std::vector<float> pyramid = {
            // pos              // uv     // normal 
            -1.f,-1.f,-0.6f,   0.0f,0.0f,  0,0,0,
             0.f,-1.f, 1.0f,   0.5f,0.0f,  0,0,0,
             1.f,-1.f,-0.6f,   1.0f,0.0f,  0,0,0,
             0.f, 1.f, 0.0f,   0.5f,1.0f,  0,0,0
        };
        std::vector<unsigned> pidx = {0,3,1, 1,3,2, 2,3,0, 0,1,2};
        
        CalculateAverageNormals(pyramid, pidx, /*vertexLen*/8, /*normalOffset*/5);

        auto m = std::make_unique<Mesh>();
        m->Create(pyramid, pidx, /*strideFloats*/8);
        meshes_.push_back(std::move(m));
    }

    {
        // Пол
        std::vector<float> floor = {
            -10,0,-10,  0,0,    0,1,0,
             10,0,-10, 10,0,    0,1,0,
            -10,0, 10,  0,10,   0,1,0,
             10,0, 10, 10,10,   0,1,0,
        };
        std::vector<unsigned> fidx = {0,2,1, 1,2,3};

        auto m = std::make_unique<Mesh>();
        m->Create(floor, fidx, /*strideFloats*/8);
        meshes_.push_back(std::move(m));
    }
    
    {
        auto tex1 = std::make_unique<Texture>("assets/textures/brick.png"); tex1->Load();  textures_.push_back(std::move(tex1));
        auto tex2 = std::make_unique<Texture>("assets/textures/dirt.png");  tex2->Load();  textures_.push_back(std::move(tex2));
        auto tex3 = std::make_unique<Texture>("assets/textures/plain.png"); tex3->Load();  textures_.push_back(std::move(tex3));

        materials_.push_back(MaterialFactory::CreateShinyMaterial());
        materials_.push_back(MaterialFactory::CreateDullMaterial());
    }

#ifdef USE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_->GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 450");
#endif
}

void GLApplication::DrawUI()
{
#ifdef USE_IMGUI
    ImGui::Begin("Info");
    ImGui::Text("FPS: %.1f", clock_.get().fps);
    ImGui::Text("WASD / Mouse. TAB toggles capture.");
    ImGui::End();

    ImGuiHandler::DrawDirectionalLightGui(dirLight_);
    ImGuiHandler::DrawPointLightsGui(pointLights_, (int)Shader::MAX_POINT_LIGHTS);
#endif
}

void GLApplication::RenderFrame()
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shader_->Id())
        shader_->BindSamplerUnit("theTexture", 0);
    
    const GLuint pid = shader_->Id();

    shader_->Use();

    glProgramUniformMatrix4fv(pid, shader_->GetUniformProj(), 1, GL_FALSE, glm::value_ptr(projection_));
    glProgramUniformMatrix4fv(pid, shader_->GetUniformView(),  1, GL_FALSE, glm::value_ptr(camera_->GetViewMatrix()));
    const auto eye = camera_->GetPosition();
    glProgramUniform3f(pid, shader_->GetUniformEyePos(), eye.x, eye.y, eye.z);
    
    dirLight_.UseLightDSA(pid, shader_->GetDirectionalLightUniforms());

    const int count = std::min<int>((int)pointLights_.size(), Shader::MAX_POINT_LIGHTS);
    glProgramUniform1i(pid, shader_->UniPointLightCount, count);
    for (int i = 0; i < count; ++i) {
        if (pointLights_[i])
            pointLights_[i]->UseLightDSA(pid, shader_->GetPointLightUniforms(i));
    }
    
    glm::mat4 model(1.0f);
    model = glm::translate(model, {0.f, 0.f, -2.5f});
    glProgramUniformMatrix4fv(pid, shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[0]->Use(0);
    materials_[0]->UseDSA(pid, shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[0]->Draw();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, {0.f, 4.f, -2.5f});
    glProgramUniformMatrix4fv(pid, shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[1]->Use(0);
    materials_[1]->UseDSA(pid, shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[0]->Draw();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, {0.f, -2.f, -2.5f});
    glProgramUniformMatrix4fv(pid, shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[2]->Use(0);
    materials_[0]->UseDSA(pid, shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[1]->Draw();
}

void GLApplication::Run()
{
    CreateScene();

    bool prevTab = false;
    while (!glfwWindowShouldClose(window_->GetWindow())) {
        glfwPollEvents();

#ifdef USE_IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        auto& io = ImGui::GetIO();
        const bool uiMouse = io.WantCaptureMouse;
        const bool uiKey   = io.WantCaptureKeyboard || io.WantTextInput;
#else
        const bool uiMouse = false, uiKey = false;
#endif
        
        bool tab = window_->GetKeys()[GLFW_KEY_TAB];
        if (tab && !prevTab) {
            gameMode_ = !gameMode_;
            glfwSetInputMode(window_->GetWindow(), GLFW_CURSOR,
                gameMode_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
        prevTab = tab;
        
        clock_.beginFrame();
        auto st = clock_.get();
        
        if (gameMode_ && !uiKey) {
            camera_->KeyControl(window_->GetKeys(), static_cast<float>(st.deltaSec));
        }
        if (gameMode_ && !uiMouse) {
            camera_->MouseControl(window_->GetDeltaX(), window_->GetDeltaY(),
                                  static_cast<float>(st.deltaSec));
        }
        window_->ResetDelta();
        
        RenderFrame();

#ifdef USE_IMGUI
        DrawUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
        glfwSwapBuffers(window_->GetWindow());
    }
}

void GLApplication::OnResize(int w, int h)
{
    glViewport(0, 0, std::max(1, w), std::max(1, h));
    projection_ = glm::perspective(glm::radians(45.0f),
        float(std::max(1, w)) / float(std::max(1, h)),
        0.1f, 100.0f);
}
