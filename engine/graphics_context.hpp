#pragma once

class GraphicsContext{
public:
    virtual ~GraphicsContext() = default;

    virtual void Init() = 0;
    // Swapbuffers function is basically useless here since it can't do everything that the glfwSwapBuffers function was doing singlehandedly.
    // virtual void SwapBuffers() = 0;
};
