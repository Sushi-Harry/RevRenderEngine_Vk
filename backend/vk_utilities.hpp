#pragma once

#include "vk_context.hpp"
#include "vk_renderer.hpp"
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

vk::raii::CommandBuffer beginSingleTimeCommands(vk::raii::CommandPool& _commandPool, vk::raii::Device& _device){
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = _commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(_device, allocInfo).front());
    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };
    commandBuffer.begin(beginInfo);

    return std::move(commandBuffer);
}

void endSingleTimeCommand(vk::raii::CommandBuffer &&commandBuffer, vk::raii::Queue& _graphics_queue){
    commandBuffer.end();
    
    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer
    };

    _graphics_queue.submit(submitInfo, nullptr);
    _graphics_queue.waitIdle();
}

uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice& physicalDevice){
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties){
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, VulkanContext* context){
    vk::BufferCreateInfo bufferInfo{
        .size = size, 
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    vk::raii::Buffer buffer = vk::raii::Buffer(context->GetDevice(), bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocationInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties, context->GetPhysicalDevice())
    };
    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(context->GetDevice(), allocationInfo);
    buffer.bindMemory(*bufferMemory, 0);
    return {std::move(buffer), std::move(bufferMemory)};
}

vk::raii::ImageView CreateImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevel, vk::raii::Device& device) {
    vk::ImageViewCreateInfo viewInfo{
        .image = image, 
        .viewType = vk::ImageViewType::e2D,
        .format = format, 
        .subresourceRange = {.aspectMask = aspectFlags, .levelCount = mipLevel, .baseArrayLayer = 0, .layerCount = 1}
    };
    return vk::raii::ImageView(device, viewInfo);
}

static void CopyBuffer(VulkanContext* context, VulkanRenderer* renderer, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(renderer->GetCommandPool(), context->GetDevice());

    vk::BufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    endSingleTimeCommand(std::move(commandBuffer), context->GetGraphicsQueue());
}