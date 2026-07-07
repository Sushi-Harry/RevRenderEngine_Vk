#pragma once

#include "graphics_context.hpp"

struct GLFWwindow;

class OpenGLContext : public GraphicsContext{
public:
    OpenGLContext(GLFWwindow* windowHandle);

    void Init() override;
    void SwapBuffers() override;

private:
    GLFWwindow* _windowHandle;
};
