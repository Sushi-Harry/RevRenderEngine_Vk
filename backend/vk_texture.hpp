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
    void CreateImage(
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
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> CreateImage(
        uint32_t width, uint32_t height,
        uint32_t mipLevel,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );
    void CopyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);
    void CreateTextureSampler();
    void GenerateMipmaps(
        vk::raii::CommandBuffer & commandBuffer,
        vk::raii::Image &image,
        vk::Format imageFormat,
        int32_t width, int32_t height,
        uint32_t mipLevel
    );

    VulkanContext* _context;
    VulkanRenderer* _renderer;

    uint32_t _width, _height;
    uint32_t _mipLevels;
    std::string _path;
    // The basic required vulkan variables for textures
    vk::raii::Image _image = nullptr;
    vk::raii::DeviceMemory _imageMemory = nullptr;
    vk::raii::ImageView _imageView = nullptr;
    vk::raii::Sampler _sampler = nullptr;
};