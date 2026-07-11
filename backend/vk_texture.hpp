#pragma once

#include "texture.hpp"

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <string>

class VulkanContext;
class VulkanRenderer;
class VulkanTexture2D : public Texture2D {
public:
    VulkanTexture2D(VulkanContext* context, VulkanRenderer* renderer, const std::string& path);
    ~VulkanTexture2D() override = default;

    uint32_t GetWidth() const override { return _width; }
    uint32_t GetHeight() const override { return _height; }
    
    void Bind(uint32_t slot = 0) const override;

    inline const vk::raii::ImageView& GetImageView() const { return _imageView; }
    inline const vk::raii::Sampler& GetSampler() const { return _sampler; }
private:
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
    void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    VulkanContext* _context;
    VulkanRenderer* _renderer;

    uint32_t _width, _height;
    std::string _path;

    // The core Vulkan texture objects
    vk::raii::Image _image = nullptr;
    vk::raii::DeviceMemory _imageMemory = nullptr;
    vk::raii::ImageView _imageView = nullptr;
    vk::raii::Sampler _sampler = nullptr;
};