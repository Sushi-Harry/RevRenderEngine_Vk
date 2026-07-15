#include "vk_buffers.hpp"
#include "vk_renderer.hpp"
#include "vk_rendering_api.hpp"

std::shared_ptr<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
    return std::make_shared<VulkanVertexBuffer>(
        VulkanRenderingAPI::GetContext(), 
        VulkanRenderingAPI::GetRenderer(), 
        vertices, 
        size
    );
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
    return std::make_shared<VulkanIndexBuffer>(
        VulkanRenderingAPI::GetContext(), 
        VulkanRenderingAPI::GetRenderer(), 
        indices, 
        count
    );
}

// \        /       .===.
//  \      /       |     |
//   \    /        |====='
//    \  /         |     `|
//     \/  ERTEX   |_____.| UFFER

#include "vk_utilities.hpp"

VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext* context, VulkanRenderer* renderer, float* vertices, uint32_t size): _context(context) 
{
    auto [stagingBuffer, stagingBufferMemory] = createBuffer(
        size, 
        vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
        _context
    );

    void* data = stagingBufferMemory.mapMemory(0, size);
    memcpy(data, vertices, (size_t)size);
    stagingBufferMemory.unmapMemory();

    std::tie(_vertexBuffer, _vertexBufferMemory) = createBuffer(
        size, 
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 
        vk::MemoryPropertyFlagBits::eDeviceLocal, 
        _context
    );

    CopyBuffer(_context, renderer, *stagingBuffer, *_vertexBuffer, size);
}

void VulkanVertexBuffer::Bind() const {
    // Gonna leave this emptytill after I've figured out how to write this function
}

void VulkanVertexBuffer::Unbind() const {}

//  ======           .===.
//    ||            |     |
//    ||            |====='
//    ||            |     `|
// ======== NDEX    |_____.| UFFER

VulkanIndexBuffer::VulkanIndexBuffer(VulkanContext* context, VulkanRenderer* renderer, uint32_t* indices, uint32_t count)
    : _context(context), _count(count)
{
    vk::DeviceSize bufferSize = sizeof(uint32_t) * count;

    // 1. Create Staging Buffer
    auto [stagingBuffer, stagingBufferMemory] = createBuffer(
        bufferSize, 
        vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
        _context
    );

    // 2. Map and Copy Data
    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, indices, (size_t)bufferSize);
    stagingBufferMemory.unmapMemory();

    // 3. Create Device Local Buffer
    std::tie(_indexBuffer, _indexBufferMemory) = createBuffer(
        bufferSize, 
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, 
        vk::MemoryPropertyFlagBits::eDeviceLocal, 
        _context
    );

    // 4. Execute the Copy
    CopyBuffer(_context, renderer, *stagingBuffer, *_indexBuffer, bufferSize);
}

void VulkanIndexBuffer::Bind() const {}
void VulkanIndexBuffer::Unbind() const {}