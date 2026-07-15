#pragma once

#include "pipeline.hpp"

#include "pipeline.hpp"

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class VulkanContext;
class VulkanSwapchain;

class VulkanPipeline : public Pipeline {
public:
    VulkanPipeline(VulkanContext* context, VulkanSwapchain* swapchain, const PipelineSpecs& specs);
    ~VulkanPipeline() override = default;

    void Bind() override;
    const PipelineSpecs& GetSpecs() const override { return _specs; }
    inline const vk::PipelineLayout& GetPipelineLayout() const { return *_pipelineLayout; }
    inline const vk::Pipeline& GetPipeline() const { return *_pipeline; }

private:
    VulkanContext* _context;
    VulkanSwapchain* _swapchain;
    PipelineSpecs _specs;
    vk::raii::PipelineLayout _pipelineLayout = nullptr;
    vk::raii::Pipeline _pipeline = nullptr;
};