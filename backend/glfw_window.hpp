#pragma once

#include "GLFW/glfw3.h"
#include <string>
#include "window.hpp"

class glfw_window : public Window{
public:
    glfw_window(const WindowProperties& winProp);
    virtual ~glfw_window();
    void onUpdate() override;

    uint32_t getWidth() override { return _data.width; }
    uint32_t getHeight() override { return _data.height; }

    void* getGLWindow() const override { return  _window; }

private:
    virtual void Init(const WindowProperties& props);
    virtual void Shutdown();

    // The actual window pointer from glfw. Is initialized in the implementation of this class in glfw_window.cpp
    GLFWwindow* _window;

    // Acts as the window properties storing part of the class
    struct WinData{
        std::string title;
        uint32_t width, height;
    };
    WinData _data;
};