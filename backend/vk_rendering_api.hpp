#pragma once

#include "rendering_api.hpp"
#include "vk_context.hpp"
#include "vk_swapchain.hpp"
#include "vk_renderer.hpp"

struct PushConstantData {
    glm::mat4 viewProjection;
    glm::mat4 model;
};

class VulkanRenderingAPI : public RenderingAPI{
public:
    void Init() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void SetClearColor(float r, float g, float b, float a) override;
    void Clear() override;

    bool BeginFrame() override;
    void EndFrame() override;
    void DrawIndexed(uint32_t indexCount, const glm::mat4& viewProj, const glm::mat4& model) override;

    // Need these to fetch the context, swapchain and renderer variables on the fly in other classes
    static VulkanContext* GetContext();
    static VulkanSwapchain* GetSwapchain();
    static VulkanRenderer* GetRenderer();
    static const vk::raii::DescriptorSetLayout& GetTextureDescriptorLayout();
    static void SetActivePipelineLayout(vk::PipelineLayout layout);
    static vk::PipelineLayout GetActivePipelineLayout();

private:
    // DEE.              SSSSSSS\\
    // D  EE.           //      
    // D   EE           \\SSSSSS,
    // D   EE                   \\
    // DEEEE' ESCRIPTOR \\SSSSSSS" ETS
    vk::raii::DescriptorSetLayout _textureSetLayout = nullptr;
    void CreateTextureDescriptorLayout();

    vk::PipelineLayout _activePipelineLayout = nullptr;

    VulkanContext* _context;
    VulkanSwapchain* _swapchain;
    VulkanRenderer* _renderer;

    glm::vec4 _clearColor;

    vk::raii::CommandBuffer* _activeCommandBuffer = nullptr;
};