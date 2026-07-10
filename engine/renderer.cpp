#include "renderer.hpp"
#include "rendering_command.hpp"

void Renderer::Init() {
    GeneralRenderCalls::Init();
}

void Renderer::onWindowResize(uint32_t width, uint32_t height) {
    GeneralRenderCalls::SetViewport(0, 0, width, height);
}

void Renderer::BeginScene(/* Take a Camera object here later */) {
    // Set up global uniforms, view/projection matrices, etc.
}

void Renderer::EndScene() {
    // Flush the render queue, etc.
}