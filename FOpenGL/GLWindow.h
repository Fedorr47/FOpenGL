#pragma once

#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GLWindow {
public:
    GLWindow(int width, int height, const char* title)
        : width(width), height(height), title(title) {
    }

    bool Initialize();

    ~GLWindow() {
        // GLFWwindow will be destroyed by custom deleter
        glfwTerminate(); // but glfwTerminate must still be called once
    }

    GLFWwindow* GetWindow() const { return window.get(); }
    int GetBufferWidth() const { return bufferWidth; }
    int GetBufferHeight() const { return bufferHeight; }

private:
    struct GLFWwindowDeleter {
        void operator()(GLFWwindow* ptr) const {
            if (ptr) {
                glfwDestroyWindow(ptr);
            }
        }
    };

    int width;
    int height;
    int bufferWidth = 0;
    int bufferHeight = 0;
    const char* title;

    std::unique_ptr<GLFWwindow, GLFWwindowDeleter> window;
};
