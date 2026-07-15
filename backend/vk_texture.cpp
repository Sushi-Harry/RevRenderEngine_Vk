#include "vk_texture.hpp"

#include "vk_context.hpp"
#include "vk_renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vk_utilities.hpp"
#include "vk_rendering_api.hpp"

std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path){
    VulkanContext* context = VulkanRenderingAPI::GetContext();
    VulkanRenderer* renderer = VulkanRenderingAPI::GetRenderer();
    return std::make_shared<VulkanTexture2D>(context, renderer, path);
}

VulkanTexture2D::VulkanTexture2D(VulkanContext* context, VulkanRenderer* renderer, const std::string& path) : _context(context), _renderer(renderer), _path(std::move(path)){
    int width, height, nrChannels;
    stbi_uc *pixels = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
    _mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    vk::DeviceSize imageSize = width * height * 4;

    if(!pixels){
        throw std::runtime_error("Failed to load texture at: " + path);
    }

    _width = width; _height = height;

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, _context);

    void* data = stagingBufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();
    stbi_image_free(pixels);

    std::tie(_image, _imageMemory) = CreateImage(
        width, height,
        _mipLevels,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(_renderer->GetCommandPool(), _context->GetDevice());
    _renderer->TransitionImageLayout(commandBuffer, _image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor, _mipLevels);
    CopyBufferToImage(commandBuffer, stagingBuffer, _image, static_cast<uint32_t>(_width), static_cast<uint32_t>(_height));
    GenerateMipmaps(commandBuffer, _image, vk::Format::eR8G8B8A8Srgb, width, height, _mipLevels);
    endSingleTimeCommand(std::move(commandBuffer), _context->GetGraphicsQueue());

    // Creaating the image view
    _imageView = CreateImageView(_image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, _mipLevels, _context->GetDevice());

    // Creating the texture sampler
    CreateTextureSampler();

    // Getting the layout and pool requires some thinking here. (If you can't already tell, I am confused asf right now and I feel like I'm high on some weird shit or rather, I'm high on stupidity right now.)
    vk::DescriptorSetLayout layout = *VulkanRenderingAPI::GetTextureDescriptorLayout();
    vk::DescriptorSetAllocateInfo allocInfo {
        .descriptorPool = *VulkanRenderingAPI::GetRenderer()->GetDescriptorPool(),
        .descriptorSetCount = 1,
        .pSetLayouts = &layout
    };
    _descriptorSet = std::move(vk::raii::DescriptorSets(_context->GetDevice(), allocInfo).front());
    vk::DescriptorImageInfo imageInfo{
        .sampler = *_sampler,
        .imageView = *_imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = *_descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo
    };

    _context->GetDevice().updateDescriptorSets(descriptorWrite, nullptr);
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> VulkanTexture2D::CreateImage(
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
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties, _context->GetPhysicalDevice())
    };
    vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(_context->GetDevice(), allocInfo);
    image.bindMemory(*imageMemory, 0);
    return {std::move(image), std::move(imageMemory)};
}

void VulkanTexture2D::CreateImage(
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
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties, _context->GetPhysicalDevice())
    };
    imageMemory = vk::raii::DeviceMemory(_context->GetDevice(), allocInfo);
    image.bindMemory(*imageMemory, 0);
}

void VulkanTexture2D::CopyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height){
    vk::BufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1}
    };

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

void VulkanTexture2D::CreateTextureSampler(){
    vk::PhysicalDeviceProperties properties = _context->GetPhysicalDevice().getProperties();
    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0F,
        .maxLod = vk::LodClampNone
    };

    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;

    _sampler = vk::raii::Sampler(_context->GetDevice(), samplerInfo);
}

void VulkanTexture2D::GenerateMipmaps(
    vk::raii::CommandBuffer & commandBuffer,
    vk::raii::Image &image,
    vk::Format imageFormat,
    int32_t width, int32_t height,
    uint32_t mipLevel
)
{
    vk::FormatProperties formatProps = _context->GetPhysicalDevice().getFormatProperties(imageFormat);
    if(!(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)){
        throw std::runtime_error("Texture image format does not support linear blitting");
    }
    vk::ImageMemoryBarrier barrier = {
        .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        .dstAccessMask = vk::AccessFlagBits::eTransferRead,
        .oldLayout = vk::ImageLayout::eTransferDstOptimal,
        .newLayout = vk::ImageLayout::eTransferSrcOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = *image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    int32_t mipWidth = width;
    int32_t mipHeight = height;
    for(uint32_t i = 1; i < mipLevel; i++){
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
    
        vk::ImageBlit blit = {
            .srcSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = i - 1,
                .layerCount = 1
            },
            .srcOffsets = std::array<vk::Offset3D, 2>({{}, {mipWidth, mipHeight, 1}}),
            .dstSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = i,
                .layerCount = 1
            },
            .dstOffsets = std::array<vk::Offset3D, 2>({{}, {1 < mipWidth ? mipWidth / 2 : 1, 1 < mipHeight ? mipHeight / 2 : 1, 1}})
        };
        commandBuffer.blitImage(*image, vk::ImageLayout::eTransferSrcOptimal, *image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
        barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
        if(1 < mipWidth){
            mipWidth /= 2;
        }
        if (1 < mipHeight){
            mipHeight /= 2;
        }
    }
    barrier.subresourceRange.baseMipLevel = _mipLevels - 1;
    barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask                 = vk::AccessFlagBits::eShaderRead;
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
}

void VulkanTexture2D::Bind(uint32_t slot) const {
    vk::raii::CommandBuffer& commandBuffer = VulkanRenderingAPI::GetRenderer()->GetCurrentCommandBuffer();
    vk::PipelineLayout activeLayout = VulkanRenderingAPI::GetActivePipelineLayout();

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, 
        activeLayout, 
        slot, 
        {*_descriptorSet},
        nullptr
    );
}