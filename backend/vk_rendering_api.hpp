#pragma once

#include "rendering_api.hpp"
#include "vk_context.hpp"
#include "vk_swapchain.hpp"
#include "vk_renderer.hpp"

class VulkanRenderingAPI : public RenderingAPI{
public:
    void Init() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void SetClearColor(float r, float g, float b, float a) override;
    void Clear() override;

    bool BeginFrame() override;
    void EndFrame() override;
    void DrawIndexed(uint32_t indexCount) override;

private:
    VulkanContext* _context;
    VulkanSwapchain* _swapchain;
    VulkanRenderer* _renderer;

    glm::vec4 _clearColor;

    vk::raii::CommandBuffer* _activeCommandBuffer = nullptr;
};