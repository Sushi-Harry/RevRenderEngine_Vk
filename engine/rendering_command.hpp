#pragma once

#include "rendering_api.hpp"


class GeneralRenderCalls{
public:
    static void Init(){
        _render_api = RenderingAPI::Create();
        _render_api->Init();
    }
    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height){
        _render_api->SetViewport(x, y, width, height);
    }
    static void SetClearColor(float r, float g, float b, float a){
        _render_api->SetClearColor(r, g, b, a);
    }
    static void Clear(){
        _render_api->Clear();
    }
    static bool BeginFrame() {
        return _render_api->BeginFrame();
    }
    static void EndFrame(){
        _render_api->EndFrame();
    }
    static void DrawIndexed(uint32_t indexCount){
        _render_api->DrawIndexed(indexCount);
    }

private:
    static std::unique_ptr<RenderingAPI> _render_api;
};