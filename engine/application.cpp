#include "application.hpp"
#include "rendering_command.hpp"

Application* Application::_instance =nullptr;
std::unique_ptr<RenderingAPI> GeneralRenderCalls::_render_api = nullptr;

Application::Application(Window* window, const std::string& name) : _window(std::move(window)){
    // set _instance pointer to this pointer. Why? Well causes THIS is the current instance (~_~)
    _instance = this;
    GeneralRenderCalls::Init();

    // Testing before moving everything to sandbox_layer.hpp/.cpp
    _camera = new Camera3D(glm::vec3(0.0f, 0.0f, 3.0f));
    float vertices[] = {
        // Position           // UVs
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // Bottom Left
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // Bottom Right
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // Top Right
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // Top Left
    };

    uint32_t indices[] = {
        0, 1, 2, 
        2, 3, 0
    };
    _vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
    _indexBuffer = IndexBuffer::Create(indices, 6);
    BufferLayout layout = {
        { "a_Position", ShaderDataType::FLOAT3 },
        { "a_TexCoord", ShaderDataType::FLOAT2 }
    };
    _vertexBuffer->SetLayout(layout);
    _shader = Shader::Create("QuadShader", "revrender/shaders/vert.spv", "revrender/shaders/frag.spv");
    _texture = Texture2D::Create("revrender/assets/texture.jpg");
    // revrender/shaders/basic.slang

    PipelineSpecs pipelineSpec{
        ._targetShader = _shader,
        ._layout = _vertexBuffer->GetLayout(),
    };
    _pipeline = Pipeline::Create(pipelineSpec);
}

Application::~Application() {}

void Application::Close(){
    _isRunning = false;
}

void Application::Run(){

    GeneralRenderCalls::SetClearColor(0.5F, 0.0F, 0.5F, 1.0F);
    while (_isRunning) {
        _window->onUpdate();
        if (GeneralRenderCalls::BeginFrame()) {
            GeneralRenderCalls::Clear(); 
            GeneralRenderCalls::SetViewport(0, 0, _window->getWidth(), _window->getHeight());

            _pipeline->Bind();
            _vertexBuffer->Bind();
            _indexBuffer->Bind();
            _texture->Bind(0);

            glm::mat4 viewProj = glm::mat4(1.0f); 
            glm::mat4 model = glm::mat4(1.0f);
            
            GeneralRenderCalls::DrawIndexed(_indexBuffer->GetCount(), viewProj, model);

            GeneralRenderCalls::EndFrame();
            _isRunning = !_window->shouldClose();
        }
    }
}