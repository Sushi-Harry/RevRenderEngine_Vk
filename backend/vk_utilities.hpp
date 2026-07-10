#pragma once

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