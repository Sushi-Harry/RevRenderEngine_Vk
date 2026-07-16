#include <stdexcept>
#include <array>
#include "vk_rendering_api.hpp"
#include "application.hpp"

static VulkanRenderingAPI* _VulkanAPI_Instance = nullptr;

std::unique_ptr<RenderingAPI> RenderingAPI::Create(){
    return std::make_unique<VulkanRenderingAPI>();
}

const vk::raii::DescriptorSetLayout& VulkanRenderingAPI::GetTextureDescriptorLayout() {
    return _VulkanAPI_Instance->_textureSetLayout;
}

void VulkanRenderingAPI::CreateTextureDescriptorLayout() {
    vk::DescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &samplerLayoutBinding
    };

    _textureSetLayout = vk::raii::DescriptorSetLayout(_context->GetDevice(), layoutInfo);
}

void VulkanRenderingAPI::Init() {
    _VulkanAPI_Instance = this;

    GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeWindow());
    _context = new VulkanContext(windowHandle);
    _context->Init();

    _swapchain = new VulkanSwapchain(_context, windowHandle);
    _swapchain->Init();

    _renderer = new VulkanRenderer(_context, _swapchain);
    _renderer->Init();
    CreateTextureDescriptorLayout();

    if (!_context || !_swapchain || !_renderer) {
        throw std::runtime_error("VulkanRendererAPI requires initialized Context, Swapchain, and Renderer!");
    }
}

void VulkanRenderingAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (!_activeCommandBuffer) return;

    vk::Viewport viewport(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
    vk::Rect2D scissor(vk::Offset2D(x, y), vk::Extent2D(width, height));
    
    _activeCommandBuffer->setViewport(0, viewport);
    _activeCommandBuffer->setScissor(0, scissor);
}

void VulkanRenderingAPI::SetClearColor(float r, float g, float b, float a) {
    _clearColor = {r, g, b, a};
}

void VulkanRenderingAPI::Clear(){
    // leaving this empty cause I can't live without a Clear function but the clear function is technically just not required at all
}

bool VulkanRenderingAPI::BeginFrame(){
    try {
        _activeCommandBuffer = &_renderer->BeginFrame();
    } catch (const std::exception& error) {
        return false;
    }

    uint32_t imageIndex = _renderer->GetCurrentFrameIndex();
    vk::Extent2D extent = _swapchain->GetExtent2D();

    // so I'll transition the color image to attachment optimal state
    _renderer->TransitionImageLayout(*_activeCommandBuffer, _swapchain->GetImage(imageIndex), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor, 1);
    // And then the depth image to attachmeent optimal state
    _renderer->TransitionImageLayout(*_activeCommandBuffer, _swapchain->GetDepthImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageAspectFlagBits::eDepth, 1);

    vk::ClearValue clearColor {
        .color = vk::ClearColorValue{
            .float32 = {{0.0F, 0.0F, 0.0F, 1.0F}}
        }
    };
    vk::ClearValue clearDepth{
        .depthStencil = vk::ClearDepthStencilValue(1.0F, 0)
    };

    vk::RenderingAttachmentInfo depthAttachmentInfo = {
        .imageView = _swapchain->GetDepthImageView(),
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepth
    };

    vk::RenderingAttachmentInfo colorAttachmentInfo = {
        .imageView = _swapchain->GetImageView(imageIndex),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {.offset = {0, 0}, .extent = extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo
    };

    _activeCommandBuffer->beginRendering(renderingInfo);

    return true;
}

void VulkanRenderingAPI::EndFrame(){
    if(!_activeCommandBuffer) return;

    _activeCommandBuffer->endRendering();

    uint32_t imageIndex = _renderer->GetCurrentFrameIndex();
    
    _renderer->TransitionImageLayout(*_activeCommandBuffer, _swapchain->GetImage(imageIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::ImageAspectFlagBits::eColor, 1);
    
    _renderer->EndFrame();
    _activeCommandBuffer = nullptr;
}

VulkanContext* VulkanRenderingAPI::GetContext() { return _VulkanAPI_Instance->_context; }
VulkanSwapchain* VulkanRenderingAPI::GetSwapchain() { return _VulkanAPI_Instance->_swapchain; }
VulkanRenderer* VulkanRenderingAPI::GetRenderer() { return _VulkanAPI_Instance->_renderer; }
void VulkanRenderingAPI::SetActivePipelineLayout(vk::PipelineLayout layout){
    _VulkanAPI_Instance->_activePipelineLayout = layout;
}
vk::PipelineLayout VulkanRenderingAPI::GetActivePipelineLayout(){
    return _VulkanAPI_Instance->_activePipelineLayout;
}

void VulkanRenderingAPI::DrawIndexed(uint32_t indexCount, const glm::mat4& viewProj, const glm::mat4& model){
    if(!_activeCommandBuffer){
        return;
    }

    PushConstantData pushData{
        .viewProjection = viewProj,
        .model = model
    };

    _activeCommandBuffer->pushConstants<PushConstantData>(
        _activePipelineLayout, 
        vk::ShaderStageFlagBits::eVertex, 
        0, 
        pushData
    );

    _activeCommandBuffer->drawIndexed(indexCount, 1, 0, 0, 0);
}