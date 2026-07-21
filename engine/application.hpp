#pragma once

#include "buffers.hpp"
#include "camera.hpp"
#include "pipeline.hpp"
#include "texture.hpp"
#include "window.hpp"
#include <memory>
#include "layer_stack.hpp"
#include "input.hpp"

class Application{
public:
    Application(Window* window, const std::string& app_name = "RevRenderVK");
    virtual ~Application();

    // This is the main rendering loop
    void Run();
    // This will be called when the application has to be closed
    void Close();
    // Returns the address of window instance
    Window& getWindow() { return *_window; }
    // Returns current application instance's pointer
    static Application& getInstance() { return *_instance; }

    void onEvent(Event& e);

private:
    bool onWindowClose(WindowCloseEvent& e);
    bool onWindowResize(WindowResizeEvent& e);

    std::unique_ptr<Window> _window;
    bool _isRunning = true;
    static Application* _instance;
    
    LayerStack _layer_stack;

    // These variables will be moved on to sandbox_layer.hpp/cpp 
    Camera3D* _camera;
    std::shared_ptr<VertexBuffer> _vertexBuffer;
    std::shared_ptr<IndexBuffer> _indexBuffer;
    std::shared_ptr<Texture2D> _texture;
    std::shared_ptr<Shader> _shader;
    std::shared_ptr<Pipeline> _pipeline;
};

Application* CreateApplication();
