#pragma once

#include "ShaderManager.h"
#include "GLWindow.h"

class GLApplication {
public:
    GLApplication(int width, int height, const char* title)
        : window(width, height, title)
    {
        shaderManager = std::make_shared<ShaderManager>();
    }

    // Initialize window and other necessary systems
    bool Initialize() {
        return window.Initialize();
    }

    // Main application loop
    void Run();

private:
    GLWindow window;

    std::shared_ptr<ShaderManager> shaderManager;

    // Handle input (keyboard, mouse, etc.)
    void ProcessInput();

    // Update game/application state
    void Update();

    // Render the current frame
    void Render();
};