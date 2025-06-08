#include "GLWindow.h"
#include <iostream>

bool GLWindow::Initialize()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed.\n";
        return false;
    }

    // Setup GLFW window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window and wrap in unique_ptr
    GLFWwindow* rawWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!rawWindow) {
        std::cerr << "GLFW window creation failed.\n";
        glfwTerminate();
        return false;
    }

    // Custom deleter: destroy the window only, not GLFW
    window = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>(rawWindow);

    glfwGetFramebufferSize(rawWindow, &bufferWidth, &bufferHeight);
    glfwMakeContextCurrent(rawWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed.\n";
        window.reset();
        glfwTerminate();
        return false;
    }

    return true;
}
