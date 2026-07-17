#pragma once

#include "events.hpp"
#include <string>
#include <cstdint>
#include <functional>

struct WindowProperties{
    std::string _name;
    uint32_t _height, _width;

    WindowProperties(std::string name = "RevRenderEngine - Vulkan", uint32_t width = 1600, uint32_t height = 900) : _name(std::move(name)), _width(width), _height(height) {}
};

class Window{
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;
    virtual void onUpdate() = 0;
    virtual uint32_t getWidth() = 0;
    virtual uint32_t getHeight() = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback)= 0;

    virtual void* getNativeWindow() const = 0;
    virtual bool shouldClose() = 0;

    static Window* create(const WindowProperties& prop = WindowProperties());
};