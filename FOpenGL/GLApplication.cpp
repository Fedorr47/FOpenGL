#include "GLApplication.h"

void GLApplication::Run()
{
    shaderManager->CreateTriangle(); // TODO: Move it to proper place
    if (!shaderManager->CompileShaders())
    {
        return;
    }
    while (!glfwWindowShouldClose(window.GetWindow())) {
        ProcessInput();   // Handle user input
        Update();         // Update application state
        Render();         // Render the current frame

        shaderManager->UseShaderProgram();
        
        glfwSwapBuffers(window.GetWindow()); // Swap the front and back buffers
        glfwPollEvents();                    // Poll for and process events
    }
}

void GLApplication::ProcessInput()
{
    // Placeholder for input handling logic
}

void GLApplication::Update()
{
    // Placeholder for update logic (e.g., animation, physics)
}

void GLApplication::Render()
{
    // Clear the screen with a red color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // RGBA
    glClear(GL_COLOR_BUFFER_BIT);

    // Placeholder for rendering code
}

