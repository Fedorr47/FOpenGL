
#include "Window/GLWindow.h"
#include <stdexcept>
#include <iostream>

static GLWindow* gWindowForCallbacks = nullptr;

GLWindow::GLWindow(int width, int height, const char* title)
{
    if (!glfwInit()) throw std::runtime_error("glfwInit failed");

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow failed");
    }
    glfwMakeContextCurrent(window_);

    // Enable vsync
    glfwSwapInterval(1);

    // GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("glewInit failed");
    }
    
    glfwGetFramebufferSize(window_, &bufferWidth_, &bufferHeight_);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    gWindowForCallbacks = this;
    CreateCallbacks();
}

GLWindow::~GLWindow()
{
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

void GLWindow::CreateCallbacks()
{
    glfwSetKeyCallback(window_, [](GLFWwindow* win, int key, int scancode, int action, int mods){
        (void)scancode; (void)mods;
        if (gWindowForCallbacks) HandleKeys(win, key, scancode, action, mods);
    });
    glfwSetCursorPosCallback(window_, [](GLFWwindow* win, double x, double y){
        if (gWindowForCallbacks) HandleMouse(win, x, y);
    });
}

void GLWindow::HandleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
    (void)window; (void)code; (void)mode;
    if (!gWindowForCallbacks) return;
    auto& self = *gWindowForCallbacks;
    if (key >= 0 && key < (int)self.keys_.size()) {
        if (action == GLFW_PRESS)   self.keys_[key] = true;
        if (action == GLFW_RELEASE) self.keys_[key] = false;
    }
}

void GLWindow::HandleMouse(GLFWwindow* window, double xPos, double yPos)
{
    (void)window;
    if (!gWindowForCallbacks) return;
    auto& self = *gWindowForCallbacks;

    if (self.mouseFirstMoved_) {
        self.lastX_ = xPos; self.lastY_ = yPos;
        self.mouseFirstMoved_ = false;
    }
    self.xChange_ = xPos - self.lastX_;
    self.yChange_ = self.lastY_ - yPos; // invert Y for look-up
    self.lastX_ = xPos; self.lastY_ = yPos;
}
