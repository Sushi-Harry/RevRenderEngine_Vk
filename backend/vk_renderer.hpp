#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>

class VulkanContext;
class VulkanSwapchain;

class VulkanRenderer {
public:
    VulkanRenderer(VulkanContext* context, VulkanSwapchain* swapchain);
    
    void Init();
    void Cleanup();

    vk::raii::CommandBuffer& BeginFrame();
    void EndFrame();

    inline uint32_t GetCurrentFrameIndex() const { return _currentFrameIndex; }
    inline uint32_t GetCurrentImageIndex() const { return _currentImageIndex; }
    inline vk::raii::CommandBuffer& GetCurrentCommandBuffer() { return _commandBuffers[_currentFrameIndex]; }

    void TransitionImageLayout(
        uint32_t                imageIndex,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask,
        vk::ImageAspectFlags image_aspect_mask);

    void TransitionImageLayout(vk::raii::CommandBuffer &commandBuffer,
        const vk::Image &image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::ImageAspectFlags image_aspect_mask,
        uint32_t mipLevels);

    void TransitionImageLayout(
        const vk::Image&  image,
        vk::ImageLayout         old_layout,
        vk::ImageLayout         new_layout,
        vk::AccessFlags2        src_access_mask,
        vk::AccessFlags2        dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask,
        vk::ImageAspectFlags    aspect_mask);

private:
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    VulkanContext* _context;
    VulkanSwapchain* _swapchain;

    vk::raii::CommandPool _commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> _commandBuffers;

    std::vector<vk::raii::Semaphore> _imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
    std::vector<vk::raii::Fence> _inFlightFences;

    uint32_t _currentFrameIndex = 0;
    uint32_t _currentImageIndex = 0;
    bool _isFrameStarted = false;

    const int MAX_FRAMES_IN_FLIGHT = 2;
};