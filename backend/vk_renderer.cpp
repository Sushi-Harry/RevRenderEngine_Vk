#include "vk_renderer.hpp"
#include "vk_context.hpp"
#include "vk_swapchain.hpp"
#include "vk_utilities.hpp"

VulkanRenderer::VulkanRenderer(VulkanContext* context, VulkanSwapchain* swapchain) : _context(context), _swapchain(swapchain) {}

void VulkanRenderer::Init() {
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
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

    if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal){
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal){
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage      = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }else{
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