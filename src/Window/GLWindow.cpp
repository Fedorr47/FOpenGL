
#include "Window/GLWindow.h"
#include <stdexcept>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static GLWindow* gWindowForCallbacks = nullptr;

static void GLFWErrorCallback(int code, const char* desc) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

GLWindow::GLWindow(int width, int height, const char* title)
{
    glfwSetErrorCallback(GLFWErrorCallback);
    if (!glfwInit()) throw std::runtime_error("glfwInit failed");
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow failed");
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // vsync
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("Failed to load GL functions via GLAD");
    }
    
    glfwGetFramebufferSize(window_, &bufferWidth_, &bufferHeight_);
    glViewport(0, 0, bufferWidth_, bufferHeight_);
    
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(
        [](GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* msg, const void*) {
            if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
                std::fprintf(stderr, "[GL] %s (type=%u, sev=%u)\n", msg, type, severity);
        }, nullptr);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
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
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int w, int h){
    if (gWindowForCallbacks) {
        gWindowForCallbacks->bufferWidth_  = w;
        gWindowForCallbacks->bufferHeight_ = h;
        glViewport(0,0,std::max(1,w),std::max(1,h));
        if (gWindowForCallbacks->onResize) gWindowForCallbacks->onResize(w,h);
    }
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

