#include "vk_swapchain.hpp"
#include "vk_context.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <limits>
#include <algorithm>

VulkanSwapchain::VulkanSwapchain(VulkanContext* ctx, GLFWwindow* windowHandle) : _context(ctx), _windowHandle(windowHandle) {}

void VulkanSwapchain::Init(){
    CreateSwapchain();
    CreateImageViews();
    CreateDepthResources();
}

void VulkanSwapchain::Recreate(){
    int width = 0, height = 0;
    glfwGetFramebufferSize(_windowHandle, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_windowHandle, &width, &height);
        glfwWaitEvents();
    }

    _context->GetDevice().waitIdle();

    Cleanup();
    Init();
}

void VulkanSwapchain::Cleanup(){
    _depthImageView = nullptr;
    _depthImage = nullptr;
    _depthImageMemory = nullptr;
    _imageViews.clear();
    _swapchain = nullptr;
}

uint32_t VulkanSwapchain::AcquireNextImage(const vk::raii::Semaphore& presentCompleteSemaphore){
    vk::Result result;
    uint32_t imageIndex;

    try{
        std::tie(result, imageIndex) = _swapchain.acquireNextImage(
            std::numeric_limits<uint64_t>::max(),
            *presentCompleteSemaphore,
            nullptr
        );
    }catch(const vk::OutOfDateKHRError error){
        Recreate();
        return std::numeric_limits<uint64_t>::max();
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    return imageIndex;
}

void VulkanSwapchain::Present(const vk::raii::Semaphore& renderFinishedSemaphore, uint32_t imageIndex){
    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*_swapchain,
        .pImageIndices = &imageIndex
    };

    vk::Result result;
    try{
        result = _context->GetGraphicsQueue().presentKHR(presentInfo);
    }catch(const vk::OutOfDateKHRError& error){
        result = vk::Result::eErrorOutOfDateKHR;
    }

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        Recreate();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image!");
    }
}

void VulkanSwapchain::CreateSwapchain(){
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = _context->GetPhysicalDevice().getSurfaceCapabilitiesKHR(_context->GetSurface());
    _extent = chooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = chooseSwapMinImgCount(surfaceCapabilities);
    std::vector<vk::SurfaceFormatKHR> availableFormats = _context->GetPhysicalDevice().getSurfaceFormatsKHR(_context->GetSurface());
    _surfaceFormat = chooseSwapSurfaceFormat(availableFormats);
    std::vector<vk::PresentModeKHR> availablePresentModes = _context->GetPhysicalDevice().getSurfacePresentModesKHR(_context->GetSurface());
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

    vk::SwapchainCreateInfoKHR swapchainCreateInfo {
        .surface = _context->GetSurface(),
        .minImageCount = minImageCount,
        .imageFormat = _surfaceFormat.format,
        .imageColorSpace = _surfaceFormat.colorSpace,
        .imageExtent = _extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = chooseSwapPresentMode(availablePresentModes),
        .clipped = true
    };

    _swapchain = vk::raii::SwapchainKHR(_context->GetDevice(), swapchainCreateInfo);
    _images = _swapchain.getImages();
}

void VulkanSwapchain::CreateImageViews(){
    _images.reserve(_images.size());
    for(auto &image : _images){
        _imageViews.emplace_back(createImageView(image, _surfaceFormat.format, vk::ImageAspectFlagBits::eColor, 1));
    }
}

void VulkanSwapchain::CreateDepthResources(){
    vk::Format depthFormat = findDepthFormat();
    std::tie(_depthImage, _depthImageMemory) = createImage(_extent.width, _extent.height, 1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _depthImageView = createImageView(_depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> VulkanSwapchain::createImage(
    uint32_t width, uint32_t height,
    uint32_t mipLevel,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties
)
{
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mipLevel,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage, 
        .sharingMode = vk::SharingMode::eExclusive
    };
    vk::raii::Image image = vk::raii::Image(_context->GetDevice(), imageInfo);
    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(_context->GetDevice(), allocInfo);
    image.bindMemory(*imageMemory, 0);
    return {std::move(image), std::move(imageMemory)};
}
void VulkanSwapchain::createImage(
    uint32_t width, uint32_t height,
    uint32_t mipLevel,
    vk::SampleCountFlagBits numSamples,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::raii::Image& image,
    vk::raii::DeviceMemory& imageMemory
)
{
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mipLevel,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = tiling, 
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    image = vk::raii::Image(_context->GetDevice(), imageInfo);
    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
    };
    imageMemory = vk::raii::DeviceMemory(_context->GetDevice(), allocInfo);
    image.bindMemory(*imageMemory, 0);
}

vk::Format VulkanSwapchain::findDepthFormat(){
    return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
    vk::ImageTiling::eOptimal,
    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
vk::Format VulkanSwapchain::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features){
    for (const auto format : candidates) {
        vk::FormatProperties props = _context->GetPhysicalDevice().getFormatProperties(format);

        if (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
            ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)))
        {
        return format;
        }

        throw std::runtime_error("Failed to find supported format.");
    }
}
vk::raii::ImageView VulkanSwapchain::createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevel) const {
    vk::ImageViewCreateInfo viewInfo{
        .image = image, 
        .viewType = vk::ImageViewType::e2D,
        .format = format, 
        .subresourceRange = {.aspectMask = aspectFlags, .levelCount = mipLevel, .baseArrayLayer = 0, .layerCount = 1}
    };
    return vk::raii::ImageView(_context->GetDevice(), viewInfo);
}
vk::SurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats){
    assert(!availableFormats.empty());
    const auto formatIt = std::ranges::find_if(availableFormats, [](const auto& format){
        return (format.format == vk::Format::eB8G8R8A8Srgb) && (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
    });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}
vk::PresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes){
    return vk::PresentModeKHR::eFifo;
}
vk::Extent2D VulkanSwapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities){
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(_windowHandle, &width, &height);
    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}
uint32_t VulkanSwapchain::chooseSwapMinImgCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities){
    auto minImgCount = std::max(3u, surfaceCapabilities.minImageCount);
    if((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImgCount)){
        minImgCount = surfaceCapabilities.maxImageCount;
    }
    return minImgCount;
}
uint32_t VulkanSwapchain::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties){
    vk::PhysicalDeviceMemoryProperties memProperties = _context->GetPhysicalDevice().getMemoryProperties();
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties){
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}