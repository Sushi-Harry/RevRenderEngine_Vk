#pragma once

#include "window.hpp"
#include <memory>

class Application{
public:
    Application(std::unique_ptr<Window> window, const std::string& app_name = "RevRenderVK");
    virtual ~Application();

    // This is the main rendering loop
    void Run();
    // This will be called when the application has to be closed
    void Close();
    // Returns the address of window instance
    Window& getWindow() { return *_window; }
    // Returns current application instance's pointer
    static Application& getInstance() { return *_instance; }

private:
    std::unique_ptr<Window> _window;
    bool _isRunning = true;
    static Application* _instance;
};

Application* CreateApplication();
