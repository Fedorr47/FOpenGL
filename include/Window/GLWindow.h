
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <span>

class GLWindow {
public:
    GLWindow(int width, int height, const char* title);
    ~GLWindow();

    GLFWwindow* GetWindow() const { return window_; }
    int GetBufferWidth() const { return bufferWidth_; }
    int GetBufferHeight() const { return bufferHeight_; }

    // Input helpers
    std::span<const bool> GetKeys() const { return std::span<const bool>(keys_); }
    double GetDeltaX() const { return xChange_; }
    double GetDeltaY() const { return yChange_; }
    void ResetDelta() { xChange_ = yChange_ = 0.0; }

private:
    static void HandleKeys(GLFWwindow* window, int key, int code, int action, int mode);
    static void HandleMouse(GLFWwindow* window, double xPos, double yPos);
    void CreateCallbacks();

private:
    GLFWwindow* window_ = nullptr;
    int bufferWidth_ = 0;
    int bufferHeight_ = 0;

    std::array<bool, 1024> keys_{};
    double lastX_ = 0.0, lastY_ = 0.0;
    double xChange_ = 0.0, yChange_ = 0.0;
    bool mouseFirstMoved_ = true;
};
