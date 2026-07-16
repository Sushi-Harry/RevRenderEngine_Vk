#include "vk_renderer.hpp"
#include "vk_context.hpp"
#include "vk_swapchain.hpp"
#include "vk_utilities.hpp"

VulkanRenderer::VulkanRenderer(VulkanContext* context, VulkanSwapchain* swapchain) : _context(context), _swapchain(swapchain) {}

void VulkanRenderer::Init() {
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
    CreateDescriptorPool();
}

void VulkanRenderer::Cleanup(){
    _context->GetDevice().waitIdle();
    
    _imageAvailableSemaphores.clear();
    _renderFinishedSemaphores.clear();
    _inFlightFences.clear();
    _commandBuffers.clear();
    _commandPool = nullptr;
}

void VulkanRenderer::TransitionImageLayout(
    uint32_t                imageIndex,
   vk::ImageLayout         old_layout,
   vk::ImageLayout         new_layout,
   vk::AccessFlags2        src_access_mask,
   vk::AccessFlags2        dst_access_mask,
   vk::PipelineStageFlags2 src_stage_mask,
   vk::PipelineStageFlags2 dst_stage_mask,
    vk::ImageAspectFlags image_aspect_mask)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
	    .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _swapchain->getImages()[_currentImageIndex],
        .subresourceRange = {
            .aspectMask = image_aspect_mask,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };

    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    _commandBuffers[_currentFrameIndex].pipelineBarrier2(dependencyInfo);
}
void VulkanRenderer::TransitionImageLayout(
    vk::raii::CommandBuffer &commandBuffer,
    const vk::Image &image,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::ImageAspectFlags image_aspect_mask,
    uint32_t mipLevels)
{
    vk::ImageMemoryBarrier barrier{
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = {
            .aspectMask = image_aspect_mask,
            .baseMipLevel = 0,
            .levelCount     = mipLevels,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    // if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal){
    //     barrier.srcAccessMask = {};
    //     barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    //     sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    //     destinationStage = vk::PipelineStageFlagBits::eTransfer;
    // }else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal){
    //     barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    //     barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    //     sourceStage      = vk::PipelineStageFlagBits::eTransfer;
    //     destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    // }else{
    //     throw std::invalid_argument("Unsupported layout transition");
    // }

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } 
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } 
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    } 
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    } 
    else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
    } 
    else {
        throw std::invalid_argument("Unsupported layout transition");
    }
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}
void VulkanRenderer::TransitionImageLayout(
    const vk::Image&  image,
    vk::ImageLayout         old_layout,
    vk::ImageLayout         new_layout,
    vk::AccessFlags2        src_access_mask,
    vk::AccessFlags2        dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask,
    vk::ImageAspectFlags    aspect_mask)
{
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(_commandPool, _context->GetDevice());

    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect_mask,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    commandBuffer.pipelineBarrier2(dependencyInfo);
    endSingleTimeCommand(std::move(commandBuffer), _context->GetGraphicsQueue());
}

void VulkanRenderer::CreateDescriptorPool(){
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        {vk::DescriptorType::eUniformBuffer, 10},
        {vk::DescriptorType::eCombinedImageSampler, 10}
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 20, // Total number of individual sets we can allocate
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
    _descriptorPool = vk::raii::DescriptorPool(_context->GetDevice(), poolInfo);
}

void VulkanRenderer::CreateCommandPool(){
    vk::CommandPoolCreateInfo poolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = _context->GetGraphicsQueueIndex()
    };

    _commandPool = vk::raii::CommandPool(_context->GetDevice(), poolInfo);
}

void VulkanRenderer::CreateCommandBuffers(){
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = _commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
    };

    _commandBuffers = vk::raii::CommandBuffers(_context->GetDevice(), allocInfo);
}

void VulkanRenderer::CreateSyncObjects(){
    _imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
        
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled 
    };

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _imageAvailableSemaphores.emplace_back(_context->GetDevice(), semaphoreInfo);
        _renderFinishedSemaphores.emplace_back(_context->GetDevice(), semaphoreInfo);
        _inFlightFences.emplace_back(_context->GetDevice(), fenceInfo);
    }
}

// Main functions
vk::raii::CommandBuffer& VulkanRenderer::BeginFrame(){
    if(_isFrameStarted){
        throw std::runtime_error("Called BeginFrame while in frame");
    }

    auto waitResult = _context->GetDevice().waitForFences({*_inFlightFences[_currentFrameIndex]}, vk::True, UINT64_MAX);
    if (waitResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for in-flight fence");
    }

    _currentImageIndex = _swapchain->AcquireNextImage(_imageAvailableSemaphores[_currentFrameIndex]);

    if (_currentImageIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Swapchain recreated, skipping frame"); 
    }

    _context->GetDevice().resetFences({*_inFlightFences[_currentFrameIndex]});

    vk::raii::CommandBuffer& commandBuffer = _commandBuffers[_currentFrameIndex];
    commandBuffer.reset();

    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    _isFrameStarted = true;
    return commandBuffer;

    // Here's how it flows
    /*
        first, Wait for the previous frame to finish
        then acquire the next image from swapchain class (If it returns the max integer, it means the swapchain was just recreated)
        reset the fence if we are submitting work
        reset and begin the command buffer
    */
}

void VulkanRenderer::EndFrame(){
    if(!_isFrameStarted){
        throw std::runtime_error("Can't call EndFrame while not in frame");
    }
    vk::raii::CommandBuffer& commandBuffer = _commandBuffers[_currentFrameIndex];

    commandBuffer.end();
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*_imageAvailableSemaphores[_currentFrameIndex],
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*_renderFinishedSemaphores[_currentFrameIndex]
    };

    _context->GetGraphicsQueue().submit(submitInfo, *_inFlightFences[_currentFrameIndex]);
    _swapchain->Present(_renderFinishedSemaphores[_currentFrameIndex], _currentImageIndex);
    
    _isFrameStarted = false;
    _currentFrameIndex = (_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}