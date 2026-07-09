#pragma once

#include "graphics_context.hpp"

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <ranges>

struct GLFWwindow;

class VulkanContext : public GraphicsContext{
public:
    VulkanContext(GLFWwindow* windowHandle);

    void Init() override;
    // Swapbuffers function is basically useless here since it can't do everything that the glfwSwapBuffers function was doing singlehandedly. 
    // All of the boilerplate has been moved to the VulkanSwapchain class to keep things structured and manageable
    // void SwapBuffers() override;

    vk::raii::PhysicalDevice& GetPhysicalDevice() { return _physicalDevice; }
    vk::raii::Device& GetDevice() { return _device; }
    vk::raii::Queue& GetGraphicsQueue() { return _queue; }
    vk::raii::SurfaceKHR& GetSurface() { return _surface; }

private:
    GLFWwindow* _windowHandle;

    vk::raii::Context _context;
    vk::raii::Instance _instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
    vk::raii::SurfaceKHR _surface = nullptr;
    vk::raii::PhysicalDevice _physicalDevice = nullptr;
    vk::raii::Device _device = nullptr;
    vk::raii::Queue _queue = nullptr;

    uint32_t _queueIndex = 0;

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT  *pCallbackData,
                                                          void *pUserData);
    
    bool isDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice);
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
};
