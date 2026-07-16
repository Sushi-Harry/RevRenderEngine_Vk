#include "application.hpp"
#include "rendering_command.hpp"

Application* Application::_instance =nullptr;
std::unique_ptr<RenderingAPI> GeneralRenderCalls::_render_api = nullptr;

Application::Application(Window* window, const std::string& name) : _window(std::move(window)){
    // set _instance pointer to this pointer. Why? Well causes THIS is the current instance (~_~)
    _instance = this;
    GeneralRenderCalls::Init();
}

Application::~Application() {}

void Application::Close(){
    _isRunning = false;
}

void Application::Run(){

    while (_isRunning) {
        _window->onUpdate();
        if (GeneralRenderCalls::BeginFrame()) {
            GeneralRenderCalls::SetClearColor(0.5F, 0.0F, 0.5F, 1.0F);            
            GeneralRenderCalls::Clear(); 
            
            
            GeneralRenderCalls::EndFrame();

            _isRunning = !_window->shouldClose();
        }
    }
}