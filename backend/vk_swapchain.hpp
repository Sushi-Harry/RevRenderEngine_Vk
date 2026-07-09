#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>

class VulkanContext;
class GLFWwindow;

class VulkanSwapchain{
public:
    VulkanSwapchain(VulkanContext* ctx, GLFWwindow* windowHandle);

    void Init();
    void Recreate();

    // Apparently glfwSwapBuffers() function does quite a bit of heavy lifting behind the scenes so I'm starting to miss it already
    uint32_t AcquireNextImage(const vk::raii::Semaphore& presentCompleteSemaphore);
    void Present(const vk::raii::Semaphore& renderFinishedSemaphore, uint32_t imageIndex);

    inline vk::Format GetColorFormat() const { return _surfaceFormat.format; }
    inline vk::Format GetDepthFormat() const { return _depthFormat; }
    inline vk::Extent2D GetExtent2D() const { return _extent; }
    inline size_t GetImageCount() const { return _images.size(); }

    inline const vk::raii::ImageView& GetImageView(uint32_t index) const { return _imageViews[index]; }
    inline const vk::raii::ImageView& GetDepthImageView() const { return _depthImageView; }
    
    vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevel) const;
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
        uint32_t width, uint32_t height,
        uint32_t mipLevel,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );
    void createImage(
        uint32_t width, uint32_t height,
        uint32_t mipLevel,
        vk::SampleCountFlagBits numSamples,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties,
        vk::raii::Image& image,
        vk::raii::DeviceMemory& imageMemory
    );

private:
    void CreateSwapchain();
    void CreateImageViews();
    void CreateDepthResources();
    void Cleanup();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    uint32_t chooseSwapMinImgCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);
    vk::Format findDepthFormat();
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    VulkanContext* _context;
    GLFWwindow* _windowHandle;

    vk::raii::SwapchainKHR _swapchain = nullptr;
    vk::SurfaceFormatKHR _surfaceFormat;
    vk::Format _depthFormat;
    vk::Extent2D _extent;

    std::vector<vk::Image> _images;
    std::vector<vk::raii::ImageView> _imageViews;

    vk::raii::Image _depthImage = nullptr;
    vk::raii::DeviceMemory _depthImageMemory = nullptr;
    vk::raii::ImageView _depthImageView = nullptr;
};