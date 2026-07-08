#include "vk_context.hpp"

#include "GLFW/glfw3.h"
#include <iostream>

#ifdef NDEBUG
constexpr bool enablevalidation = true;
#else
constexpr bool enablevalidation = false;
#endif

// As I'm writing this, I've got a random asmongold video running in the background about some weird youtuber getting arrested called "Jidion" or something.
// I'm mentioning this cause damn are people dumb bro imaging trash talking to the police when you know you're on camera doing the wrong thing with no sign of any shame.
// I don;t know who Jidion is, this isthe first time I'm hearing about that guy but damn am I impressed by the fact that even dumbasses like them can manage to garner views on youtube.
//----------------------------------------------------------------------//
// --------------------------HELPER FUNCTIONS-------------------------- //
//----------------------------------------------------------------------//

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> getRequiredInstanceExtensions(){
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (enablevalidation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

VulkanContext::VulkanContext(GLFWwindow* windowHandle) : _windowHandle(windowHandle) {
    if(!_windowHandle){
        throw std::runtime_error("ERROR::VULKAN_CONTEXT::NULL_WINDOW_HANDLE");
    }
}

void VulkanContext::Init() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
}

void VulkanContext::CreateInstance(){
    std::vector<const char*> requiredLayers;
    if(enablevalidation){
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    auto layerProperties = _context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](const auto &requiredLayer){
        return std::ranges::none_of(layerProperties, [requiredLayer](const auto &layerProperty){
            return strcmp(layerProperty.layerName, requiredLayer) == 0;
        });
    });
    if(unsupportedLayerIt != requiredLayers.end()){
        throw std::runtime_error("Required Layer Not Supported: " + std::string(*unsupportedLayerIt));
    }

    auto requiredExtensions = getRequiredInstanceExtensions();
    // Checking if the required extensions are suppoprted
    auto extensionProperties = _context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions, [&extensionProperties](const auto& requiredExtension){
        return std::ranges::none_of(extensionProperties, [requiredExtension](const auto& extensionProperty){
            return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
        });
    });
    if(unsupportedPropertyIt != requiredExtensions.end()){
        throw std::runtime_error("Required Extension Not Supported: " + std::string(*unsupportedPropertyIt));
    }

    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    uint32_t glfwExtCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };
    _instance = vk::raii::Instance(_context, createInfo);

}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanContext::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                            vk::DebugUtilsMessageTypeFlagsEXT type,
                                                            const vk::DebugUtilsMessengerCallbackDataEXT  *pCallbackData,
                                                            void *pUserData)
{
    std::cerr << "Validation Layer: type " << vk::to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
    return vk::False;
}

void VulkanContext::SetupDebugMessenger(){
    if(!enablevalidation) return;
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
                                                                        .messageType     = messageTypeFlags,
                                                                        .pfnUserCallback = &debugCallback};
    _debugMessenger = _instance.createDebugUtilsMessengerEXT( debugUtilsMessengerCreateInfoEXT );
}

void VulkanContext::CreateSurface(){
    VkSurfaceKHR surface;
    if(glfwCreateWindowSurface(*_instance, _windowHandle, nullptr, &surface) != 0){
        throw std::runtime_error("Failed to create window surface");
    }
    _surface = vk::raii::SurfaceKHR(_instance, surface);
}

void VulkanContext::PickPhysicalDevice(){
    std::vector<vk::raii::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
    auto devIter = std::ranges::find_if(physicalDevices, [&](const auto& physicalDevice){
        return isDeviceSuitable(physicalDevice);
    });
    if(devIter == physicalDevices.end()){
        throw std::runtime_error("Failed to Find a Suitable GPU.");
    }
    _physicalDevice = *devIter;
}

bool VulkanContext::isDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice){
    bool supports_vk1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const auto& qfp){
        return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    });

    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(requiredDeviceExtension, [&availableDeviceExtensions](const auto& requiredDeviceExtension){
        return std::ranges::any_of(availableDeviceExtensions, [requiredDeviceExtension](const auto& availableDeviceExtension){
            return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
        });
    });

    auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                        vk::PhysicalDeviceVulkan11Features,
                                                        vk::PhysicalDeviceVulkan13Features,
                                                        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                    features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                    features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                                    features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                                    features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
    return supports_vk1_3 && supportsRequiredFeatures && supportsGraphics && supportsAllRequiredExtensions;
    
}

void VulkanContext::CreateLogicalDevice(){
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
    auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](const auto& qfp){
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
    });
    _queueIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    float queuePriority = 1.0F;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = _queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
        {.features = {.samplerAnisotropy = vk::True}},
        {.shaderDrawParameters = true},
        {.synchronization2 = vk::True, .dynamicRendering = true},
        {.extendedDynamicState = true}
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data()
    };

    _device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
    _queue = vk::raii::Queue(_device, _queueIndex, 0);
}

void VulkanContext::SwapBuffers(){
    // Man I have no god damn idea how to fill all the swapchain image swapping and stuff inside this function.
    // Here's what I'm gonna do: make separate swapchain and other required classes and then figure out something along that path
}