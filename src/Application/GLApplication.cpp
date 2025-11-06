
#include "Application/GLApplication.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stack>

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
    // Camera
    camera_ = std::make_shared<Camera>(
        glm::vec3(0.0f, 1.0f, 5.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, 0.0f, 5.0f, 0.1f);

    projection_ = glm::perspective(glm::radians(45.0f),
        float(window_->GetBufferWidth())/float(window_->GetBufferHeight()),
        0.1f, 100.0f);

    // Shader
    shader_ = std::make_unique<Shader>();
    if (!shader_->CreateFromFiles("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "Failed to create shader.\n";
    }

    // Lights
    DirectionalLightProperties dlp;
    dlp.Colour = {1,1,1};
    dlp.AmbientIntensity = 0.25f;
    dlp.DiffuseIntensity = 0.8f;
    dlp.Direction = { -0.5f, -1.0f, -0.3f };
    dirLight_ = DirectionalLight(dlp);
    
    PointLightProperties plp;
    plp.Colour = {1,0,0}; plp.Position = {-3,1,0};
    pointLights_.push_back(std::make_shared<PointLight>(plp));
    plp.Colour = {0,0,1}; plp.Position = { 3,1,0};
    pointLights_.push_back(std::make_shared<PointLight>(plp));

    // Geometry: a simple floor quad and a pyramid like in your example
    std::vector<float> pyramid = {
        // pos             // uv     // normal (placeholder; not used in VS, we compute per-vertex usually)
        -1.f,-1.f,-0.6f,   0,0,     0,0,0,
         0.f,-1.f, 1.0f,   0.5,0,   0,0,0,
         1.f,-1.f,-0.6f,   1,0,     0,0,0,
         0.f, 1.f, 0.0f,   0.5,1,   0,0,0
    };
    std::vector<unsigned> pidx = {0,3,1, 1,3,2, 2,3,0, 0,1,2};
    ComputeAverageNormals(pyramid, pidx, 8, 5);

    auto m1 = std::make_unique<Mesh>(); m1->Create(pyramid, pidx, 8);
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

    // Textures / materials
    auto tex1 = std::make_unique<Texture>("assets/textures/brick.png");
    tex1->Load(); textures_.push_back(std::move(tex1));
    auto tex2 = std::make_unique<Texture>("assets/textures/dirt.png");
    tex2->Load(); textures_.push_back(std::move(tex2));
    auto tex3 = std::make_unique<Texture>("assets/textures/plain.png");
    tex3->Load(); textures_.push_back(std::move(tex3));

    materials_.push_back(std::make_shared<Material>(1.0f,64.0f)); // shiny
    materials_.push_back(std::make_shared<Material>(0.3f,8.0f));  // dull

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
    ImGui::Text("FPS: %.1f", clock_.get().fps);
    ImGui::Text("WASD/mouse; TAB toggles mouse capture");
    ImGui::End();

    ImGuiHandler::DrawDirectionalLightGui(dirLight_);
    int CurrentLightPointsNum = Shader::MAX_POINT_LIGHTS;
    ImGuiHandler::DrawPointLightsGui(pointLights_,(int)Shader::MAX_POINT_LIGHTS);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void GLApplication::RenderFrame()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_->Use();
    glUniformMatrix4fv(shader_->GetUniformProj(), 1, GL_FALSE, glm::value_ptr(projection_));
    glUniformMatrix4fv(shader_->GetUniformView(), 1, GL_FALSE, glm::value_ptr(camera_->GetViewMatrix()));
    glUniform3f(shader_->GetUniformEyePos(), camera_->GetPosition().x, camera_->GetPosition().y, camera_->GetPosition().z);

    // Lights
    dirLight_.UseLight(shader_->DirLight);
    glUniform1i(shader_->UniPointLightCount, (GLint)pointLights_.size());
    for (int i = 0; i<static_cast<int>(pointLights_.size()) && i < Shader::MAX_POINT_LIGHTS; ++i)
    {
        if (pointLights_[i] == nullptr)
        {
            continue;
        }
        pointLights_[i]->UseLight(shader_->PointLights[i]);
    }
    // Draw pyramid
    glm::mat4 model(1.0f);
    model = glm::translate(model, {0,0,-2.5f});
    glUniformMatrix4fv(shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[0]->Use(GL_TEXTURE0);
    materials_[0]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[0]->Draw();

    // upper pyramid
    model = glm::mat4(1.0f);
    model = glm::translate(model, {0,4,-2.5f});
    glUniformMatrix4fv(shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[1]->Use(GL_TEXTURE0);
    materials_[1]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[0]->Draw();

    // floor
    model = glm::mat4(1.0f);
    model = glm::translate(model, {0,-2,-2.5f});
    glUniformMatrix4fv(shader_->GetUniformModel(), 1, GL_FALSE, glm::value_ptr(model));
    textures_[2]->Use(GL_TEXTURE0);
    materials_[0]->Use(shader_->GetUniformSpecularIntensity(), shader_->GetUniformShininess());
    meshes_[1]->Draw();
}

void GLApplication::Run()
{
    CreateScene();
    clock_.reset();

    bool prevTab=false;
    while (!glfwWindowShouldClose(window_->GetWindow())) {
        glfwPollEvents();

        // Edge detect TAB for mouse capture toggle
        bool tab = window_->GetKeys()[GLFW_KEY_TAB];
        if (tab && !prevTab) {
            gameMode_ = !gameMode_;
            glfwSetInputMode(window_->GetWindow(), GLFW_CURSOR,
                gameMode_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
        prevTab = tab;

        clock_.beginFrame();
        auto st = clock_.get();

#ifdef USE_IMGUI
        auto& io = ImGui::GetIO();
        bool uiMouse = io.WantCaptureMouse;
        bool uiKey = io.WantCaptureKeyboard || io.WantTextInput;
#else
        bool uiMouse = false, uiKey=false;
#endif

        if (gameMode_ && !uiKey) camera_->KeyControl(window_->GetKeys(), (float)st.deltaSec);
        if (gameMode_ && !uiMouse) camera_->MouseControl(window_->GetDeltaX(), window_->GetDeltaY(), (float)st.deltaSec);
        window_->ResetDelta();

        RenderFrame();
        DrawUI();

        glfwSwapBuffers(window_->GetWindow());
    }
}
