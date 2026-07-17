#include "vk_pipeline.hpp"
#include "vk_context.hpp"
#include "vk_shader.hpp"
#include "vk_utilities.hpp"
#include "vk_swapchain.hpp"
#include "vk_rendering_api.hpp"

std::unique_ptr<Pipeline> Pipeline::Create(const PipelineSpecs& specs) {
    VulkanContext* context = VulkanRenderingAPI::GetContext();
    VulkanSwapchain* swapchain = VulkanRenderingAPI::GetSwapchain();
    
    return std::make_unique<VulkanPipeline>(context, swapchain, specs);
}

VulkanPipeline::VulkanPipeline(VulkanContext* context, VulkanSwapchain* swapchain, const PipelineSpecs& specs) : _context(context), _swapchain(swapchain), _specs(specs){
    auto vkShader = std::static_pointer_cast<VulkanShader>(specs._targetShader);
    // Shader Stages
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vkShader->GetVertexModule(),
            .pName = "main" // The entry point of your GLSL
        },
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = vkShader->GetFragmentModule(),
            .pName = "main"
        }
    };
    // Dynamic states
    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), 
        .pDynamicStates = dynamicStates.data()
    };

    // Now here's where the code differs from the learning project I've bene using as a reference for the most part
    vk::VertexInputBindingDescription bindingDescription{
        .binding = 0,
        .stride = _specs._layout.getStride(),
        .inputRate = vk::VertexInputRate::eVertex
    };

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    uint32_t location = 0;
    for(const auto& element : _specs._layout){
        attributeDescriptions.push_back({
            .location = location,
            .binding = 0,
            .format = ShaderDataTypeToVulkanFormat(element._type),
            .offset = static_cast<uint32_t>(element._offset)
        });
        location++;
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    //Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology=vk::PrimitiveTopology::eTriangleList};

    // Viewports and Scissors
    vk::Viewport viewport{0.0F, 0.0F, static_cast<float>(_swapchain->GetExtent2D().width), static_cast<float>(_swapchain->GetExtent2D().height), 0.0F, 1.0F};
    vk::Rect2D scissor{vk::Offset2D{0, 0}, _swapchain->GetExtent2D()};
    vk::PipelineViewportStateCreateInfo viewportState {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0F
    };

    // Color Blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::False,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy, 
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };
    
    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    // Depth Stencil State
    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False
    };

    // Pipeline Layout
    vk::PushConstantRange pushConstantRange{
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(glm::mat4) * 2
    };
    vk::DescriptorSetLayout textureLayout = *VulkanRenderingAPI::GetTextureDescriptorLayout();
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &textureLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    _pipelineLayout = vk::raii::PipelineLayout(_context->GetDevice(), pipelineLayoutInfo);

    // Render Pass
    vk::Format colorFormat = VulkanRenderingAPI::GetSwapchain()->GetColorFormat();
    vk::Format depthFormat = VulkanRenderingAPI::GetSwapchain()->GetDepthFormat();

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
        {
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = *_pipelineLayout,
            .renderPass = nullptr
        },
        {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = depthFormat
        }
    };

    _pipeline = vk::raii::Pipeline(_context->GetDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

void VulkanPipeline::Bind(){
    vk::raii::CommandBuffer& commandBuffer = VulkanRenderingAPI::GetRenderer()->GetCurrentCommandBuffer();
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *_pipeline);
    VulkanRenderingAPI::SetActivePipelineLayout(*_pipelineLayout);
}