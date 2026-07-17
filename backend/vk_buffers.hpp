#pragma once
#include "buffers.hpp"

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif


class VulkanContext;
class VulkanRenderer;

class VulkanVertexBuffer : public VertexBuffer {
public:
    VulkanVertexBuffer(VulkanContext* context, VulkanRenderer* renderer, float* vertices, uint32_t size);
    ~VulkanVertexBuffer() override = default;

    void Bind() const override;
    void Unbind() const override;

    void SetData(const void* data, uint32_t size) override {}
    const BufferLayout& GetLayout() const override { return _layout; }
    void SetLayout(const BufferLayout& layout) override { _layout = layout; }

    inline const vk::Buffer& GetBuffer() const { return *_vertexBuffer; }

private:
    VulkanContext* _context;
    
    BufferLayout _layout;
    vk::raii::Buffer _vertexBuffer = nullptr;
    vk::raii::DeviceMemory _vertexBufferMemory = nullptr;
};


class VulkanIndexBuffer : public IndexBuffer {
public:
    VulkanIndexBuffer(VulkanContext* context, VulkanRenderer* renderer, uint32_t* indices, uint32_t count);
    ~VulkanIndexBuffer() override = default;

    void Bind() const override;
    void Unbind() const override;
    uint32_t GetCount() const override { return _count; }

    inline const vk::Buffer& GetBuffer() const { return *_indexBuffer; }

private:
    VulkanContext* _context;
    VulkanRenderer* _renderer;
    uint32_t _count;
    vk::raii::Buffer _indexBuffer = nullptr;
    vk::raii::DeviceMemory _indexBufferMemory = nullptr;
};