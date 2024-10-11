#include "NanoGraphics.hpp"
#include "NanoConfig.hpp"
#include "NanoError.hpp"
#include "NanoLogger.hpp"
#include "NanoUtility.hpp"
#include "NanoWindow.hpp"
#include "NanoShader.hpp"
#include "NanoGraphicsPipeline.hpp"

#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

struct QueueFamilyIndices {
    int32_t graphicsFamily = -1;
    int32_t presentFamily = -1;

    bool IsValid() { // helper function to validate queue indices
        return graphicsFamily != -1 && presentFamily != -1;
    }
};

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR capabilities{};

    std::vector<VkSurfaceFormatKHR> formats{};
    VkSurfaceFormatKHR selectedFormat{};

    std::vector<VkPresentModeKHR> presentModes{};
    VkPresentModeKHR selectedPresentMode{};

    VkExtent2D currentExtent;

    uint32_t imageCount;
};

struct NanoVKContext {
    VkInstance instance{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};

    struct QueueFamilyIndices queueIndices {};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};

    VkSurfaceKHR surface{};

    VkSwapchainKHR swapchain{};
    struct SwapChainDetails swapchainInfo {};
    std::vector<VkImage> swapchainImages{};
    std::vector<VkImageView> swapchainImageViews{};
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderpass{};

    std::vector<NanoGraphicsPipeline> graphicsPipelines{};

    VkCommandPool commandPool{};
    VkCommandBuffer commandBuffer{};

    void AddGraphicsPipeline(const NanoGraphicsPipeline& graphicsPipeline){
        graphicsPipelines.push_back(std::move(graphicsPipeline));
    }
} _NanoContext;

VkDebugUtilsMessengerEXT debugMessenger{};

// We have to look up the address of the debug callback create function ourselves using vkGetInstanceProcAddr
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// We have to look up the address of the debug callback destroy function ourselves using vkGetInstanceProcAddr
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

    std::string message{};
    ERRLevel messageLevel{};

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        messageLevel = ERRLevel::DEBUG;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        messageLevel = ERRLevel::INFO;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        messageLevel = ERRLevel::WARNING;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        messageLevel = ERRLevel::FATAL;
        break;
    default:
        messageLevel = ERRLevel::INFO;
        break;
    }

    LOG_MSG(messageLevel, "%s", pCallbackData->pMessage);
    fprintf(stderr, "Objects involved: \n");
    for (int i = 0; i < pCallbackData->objectCount; i++) {
        fprintf(stderr, "\t%s\n", pCallbackData->pObjects[i].pObjectName);
    }
    fprintf(stderr, "\n");

    if (messageLevel == ERRLevel::FATAL)
        throw std::runtime_error("Validation layer has returned fatal error");

    return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

ERR NanoGraphics::CleanUp() {
    ERR err = ERR::OK;

    vkDestroyCommandPool(_NanoContext.device, _NanoContext.commandPool, nullptr);

    for (auto framebuffer : _NanoContext.swapChainFramebuffers) {
        vkDestroyFramebuffer(_NanoContext.device, framebuffer, nullptr);
    }

    for (auto& graphicsPipeline : _NanoContext.graphicsPipelines){
        graphicsPipeline.CleanUp();
    }

    vkDestroyRenderPass(_NanoContext.device, _NanoContext.renderpass, nullptr);

    for (auto& imageView : _NanoContext.swapchainImageViews) {
        vkDestroyImageView(_NanoContext.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(_NanoContext.device, _NanoContext.swapchain, nullptr);

    vkDestroySurfaceKHR(_NanoContext.instance, _NanoContext.surface, nullptr);

    vkDestroyDevice(_NanoContext.device, nullptr);

    if (Config::enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(_NanoContext.instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(_NanoContext.instance, nullptr);

    return err;
}

static bool checkValidationLayerSupport(const char *const *validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    int indx = 0;
    while (validationLayers[indx]) {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers) {
            if (strcmp(validationLayers[indx], layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
        indx++;
    }

    return true;
}

SwapChainDetails querySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface) {
    SwapChainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

static std::vector<const char *> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // additional instance extension we may want to add
    std::vector<const char *> instanceExtensions;
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        instanceExtensions.emplace_back(glfwExtensions[i]);
    }
#if MACOS
    instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    if (Config::enableValidationLayers) {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return instanceExtensions;
    // ----------------------------------------
}

static std::vector<VkExtensionProperties> &getSupportedInstanceExtensions(std::vector<VkExtensionProperties> &extensions) {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    extensions.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

#ifdef _DEBUG
    std::cout << "available extensions:\n";
    for (const auto &extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
    std::cout.flush();
#endif

    return extensions;
}

static ERR createInstance(const char *applicationName, const char *engineName, VkInstance &instance) {
    ERR err = ERR::OK;

    if (Config::enableValidationLayers && !checkValidationLayerSupport(Config::desiredValidationLayers)) {
        LOG_MSG(ERRLevel::FATAL, "Number of Desired Layers %zu\n", Utility::SizeOf(Config::desiredValidationLayers));
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#if MACOS
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    // std::vector<VkExtensionProperties> test = {};
    // _getSupportedInstanceExtensions(test);

    std::vector<const char *> instanceExtensions = getRequiredInstanceExtensions();
    createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (Config::enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(Utility::SizeOf(Config::desiredValidationLayers));
        createInfo.ppEnabledLayerNames = Config::desiredValidationLayers;

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        return ERR::UNDEFINED;
    }
    return err;
}

static ERR setupDebugMessenger(VkInstance &instance, VkDebugUtilsMessengerEXT &debugMessenger) {
    ERR err = ERR::OK;

    if (!Config::enableValidationLayers) {
        return ERR::OK;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    VK_CHECK_THROW(CreateDebugUtilsMessengerEXT(_NanoContext.instance, &createInfo, nullptr, &debugMessenger), "Failed to create Debug Messenger\n");
    return ERR::OK; // this is never reached if we use try/catch.
}

ERR findQueueFamilies(VkPhysicalDevice device, QueueFamilyIndices &indices) {
    ERR err = ERR::NOT_FOUND;
    // Logic to find queue family indices to populate struct with

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _NanoContext.surface, &presentSupport);
            if (presentSupport)
                indices.presentFamily = i;
        }

        if (indices.IsValid()) {
            err = ERR::OK;
            return err;
        }
    }

    return err;
}

ERR checkDeviceExtensionSupport(VkPhysicalDevice device) {
    ERR err = ERR::OK;
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions{};
    int extIdx = 0;
    while (Config::deviceExtensions[extIdx]) {
        requiredExtensions.emplace(std::string(Config::deviceExtensions[extIdx]));
        extIdx++;
    }

    // make sure we go through all the list of desiredExtensions and "check off" all of them
    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    err = requiredExtensions.empty() ? ERR::OK : ERR::NOT_FOUND;
    return err;
}

int rateDeviceSuitability(const VkPhysicalDevice &device, const VkSurfaceKHR &surface, QueueFamilyIndices &queueIndices) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    // Application can't function without the required device extensions
    bool extensionsSupported = ERR::OK == checkDeviceExtensionSupport(device);
    if (!extensionsSupported) {
        return 0;
    }

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        if (!swapChainAdequate) {
            return 0; // Application can't function without an adequate swapchain extensions
        }
    }

    if (ERR::NOT_FOUND == findQueueFamilies(device, queueIndices)) {
        return 0;
    }

    return score;
}

static ERR pickPhysicalDevice(const VkInstance &instance, const VkSurfaceKHR &surface, QueueFamilyIndices &queueIndices,
                              VkPhysicalDevice &physicalDevice) {
    ERR err = ERR::OK;
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_NanoContext.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_NanoContext.instance, &deviceCount, devices.data());

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices) {
        int score = rateDeviceSuitability(device, surface, queueIndices);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    int bestScore = 0;
    for (auto &candidate : candidates) {
        if (candidate.first > 0 && candidate.first > bestScore) {
            bestScore = candidate.first;
            physicalDevice = candidates.rbegin()->second;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        err = ERR::NOT_FOUND;
        throw std::runtime_error("failed to find a suitable GPU!");
    } else {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        char deviceType[256] = {0};
        switch (deviceProperties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            strcpy_s(deviceType, "DESCRETE GPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            strcpy_s(deviceType, "INTEGRATED GPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            strcpy_s(deviceType, "VIRTUAL GPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            strcpy_s(deviceType, "CPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            strcpy_s(deviceType, "OTHER");
            break;
        default:
            strcpy_s(deviceType, "UNKNOWN");
        }
        LOG_MSG(ERRLevel::INFO, "Physical device selected: %s [%s]", deviceProperties.deviceName, deviceType);
    }

    return ERR::OK; // this is never reached if we use try/catch.
}

ERR createLogicalDevice(VkPhysicalDevice &physicalDevice, QueueFamilyIndices &indices, VkQueue &graphicsQueue, VkQueue &presentQueue,
                        VkDevice &device) {
    ERR err = ERR::OK;

    if (!indices.IsValid() && ERR::NOT_FOUND == findQueueFamilies(_NanoContext.physicalDevice, indices)) {
        throw std::runtime_error("failed to find a graphics familiy queue!");
    }

    // we know both the graphics and present indices are valid, so we don't need to worry about checking it again here
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        if (queueFamily == -1)
            continue;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    VkPhysicalDeviceFeatures deviceFeatures{}; // defaults all the features to false for now
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(Utility::SizeOf(Config::desiredValidationLayers));
    createInfo.ppEnabledExtensionNames = Config::deviceExtensions;
    if (Config::enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(Utility::SizeOf(Config::desiredValidationLayers));
        createInfo.ppEnabledLayerNames = Config::desiredValidationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    if (indices.IsValid()) {
        vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue); // The graphics queue is already created if we have successfully created
                                                                             // a logical device. This is only to retrieve the handle
        vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue); // The graphics queue is already created if we have successfully created a
                                                                           // logical device. This is only to retrieve the handle
    }

    return err;
}

ERR createSurface(VkInstance instance, GLFWwindow *window, VkSurfaceKHR &surface) {
    ERR err = ERR::OK;

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        err = ERR::INVALID;
        throw std::runtime_error("failed to create window surface!");
    }
    return err;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            LOG_MSG(ERRLevel::INFO, "swapchain format used: VK_FORMAT_B8G8R8A8_SRGB");
            LOG_MSG(ERRLevel::INFO, "swapchain colorspace used: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
            return availableFormat;
        }
    }
    return availableFormats[0]; // pick the first available format if we do not find the format desired
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            LOG_MSG(ERRLevel::INFO, "swapchain present mode used: VK_PRESENT_MODE_MAILBOX_KHR");
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        LOG_MSG(ERRLevel::INFO, "swapchain extent mode used: {%d,%d}", capabilities.currentExtent.width, capabilities.currentExtent.height);
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

ERR createSwapChain(const VkPhysicalDevice &physicalDevice, const VkDevice &device, GLFWwindow *window, const VkSurfaceKHR &surface,
                    VkSwapchainKHR &swapchain, SwapChainDetails& swapChainDetails, std::vector<VkImage> &swapchainImages) {
    ERR err = ERR::OK;
    swapChainDetails = querySwapChainSupport(physicalDevice, surface);

    swapChainDetails.selectedFormat = chooseSwapSurfaceFormat(swapChainDetails.formats);
    swapChainDetails.selectedPresentMode = chooseSwapPresentMode(swapChainDetails.presentModes);
    swapChainDetails.currentExtent = chooseSwapExtent(window, swapChainDetails.capabilities);

    swapChainDetails.imageCount = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 && swapChainDetails.imageCount > swapChainDetails.capabilities.maxImageCount) {
        swapChainDetails.imageCount = swapChainDetails.capabilities.maxImageCount;
    }

    LOG_MSG(ERRLevel::INFO, "Number of images used by the swapchain: %d", swapChainDetails.imageCount);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = swapChainDetails.imageCount;
    createInfo.imageFormat = swapChainDetails.selectedFormat.format;
    createInfo.imageColorSpace = swapChainDetails.selectedFormat.colorSpace;
    createInfo.imageExtent = swapChainDetails.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = {};
    err = findQueueFamilies(physicalDevice, indices);
    uint32_t queueFamilyIndices[2] = {};
    if (indices.IsValid()) {
        queueFamilyIndices[0] = indices.graphicsFamily;
        queueFamilyIndices[1] = indices.presentFamily;
    } else {
        LOG_MSG(ERRLevel::WARNING, "Queue Family indices is invalid. called in : createSwapChain(...)")
    }

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
        LOG_MSG(ERRLevel::WARNING, "Graphics queue family is not the same as Present queue family. Swapchain images have to be concurent")
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // TODO: If transparency is to be enabled, change this
    if (createInfo.compositeAlpha != VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        LOG_MSG(ERRLevel::INFO, "swapchain images not set to OPAQUE_BIT_KHR. Transparent window might be enabled")
    }
    createInfo.presentMode = swapChainDetails.selectedPresentMode;
    createInfo.clipped = VK_TRUE; // This deals with obstructed pixels when, for example, another window is ontop.
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &swapChainDetails.imageCount, nullptr);
    swapchainImages.resize(swapChainDetails.imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &swapChainDetails.imageCount, swapchainImages.data());

    return err;
}

ERR createSCImageViews(const VkDevice &device, const SwapChainDetails &swapchainDetails, const std::vector<VkImage> &swapchainImages,
                       std::vector<VkImageView> &swapchainImageViews) {
    ERR err = ERR::OK;
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainDetails.selectedFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

    return err;
}

ERR createGraphicsPipeline(VkDevice& device,const SwapChainDetails& swapChainDetails, const VkRenderPass& renderpass, NanoGraphicsPipeline& graphicsPipeline) {
    ERR err = ERR::OK;

    graphicsPipeline.Init(device, swapChainDetails.currentExtent);
    graphicsPipeline.AddVertShader("./src/shader/shader.vert");
    graphicsPipeline.AddFragShader("./src/shader/shader.frag");
    graphicsPipeline.AddRenderPass(renderpass);
    graphicsPipeline.Compile();

    return err;
}

ERR createRenderPass(VkDevice& device, const SwapChainDetails& swapChainDetails, VkRenderPass& renderpass){
    ERR err = ERR::OK;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainDetails.selectedFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // The following other types of attachments can be referenced by a subpass:
    // pInputAttachments: Attachments that are read from a shader
    // pResolveAttachments: Attachments used for multisampling color attachments
    // pDepthStencilAttachment: Attachment for depth and stencil data
    // pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // always add the reference to the attachment and never the attachment itself
    // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return err;
}

ERR createFramebuffer(VkDevice& device, const SwapChainDetails& swapChainDetails, const VkRenderPass& renderpass, const std::vector<VkImageView>& swapChainImageViews, std::vector<VkFramebuffer>& swapChainFramebuffers){
    ERR err = ERR::OK;
    swapChainFramebuffers.resize(swapChainDetails.imageCount);

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainDetails.currentExtent.width;
        framebufferInfo.height = swapChainDetails.currentExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
    return err;
}

ERR createCommandPool(VkDevice& device, const QueueFamilyIndices& queueFamilyIndices, VkCommandPool& commandPool){
    ERR err = ERR::OK;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT(queueFamilyIndices.graphicsFamily != -1, "Graphics family indices is not defined - createCommandPool(...)");
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    return err;
}

ERR createCommandBuffer(VkDevice& device, const VkCommandPool& commandPool, VkCommandBuffer& commandBuffer){
    ERR err = ERR::OK;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    return err;
}

ERR recordCommandBuffer(NanoGraphicsPipeline& graphicsPipeline, const VkRenderPass& renderPass, const SwapChainDetails swapChainDetails, std::vector<VkFramebuffer>& swapChainFrameBuffers, VkCommandBuffer& commandBuffer, uint32_t imageIndex) {
    ERR err = ERR::OK;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainDetails.currentExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    {
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.GetPipeline());

        // need to manually set the viewport and scissor here because we defined them as dynamic.
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainDetails.currentExtent.width);
        viewport.height = static_cast<float>(swapChainDetails.currentExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainDetails.currentExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
    }

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    return err;
}

ERR NanoGraphics::Init(NanoWindow &window) {
    ERR err = ERR::OK;
    // Here the err validation is not that useful
    // a iferr_return can be added at the end of each statement to check it's state and exit (or at least warn) if an error did occur
    err = createInstance(Config::APP_NAME,
                         Config::ENGINE_NAME,
                         _NanoContext.instance); // APP_NAME and ENGINE_NAME is defined in NanoConfig

    err = setupDebugMessenger(_NanoContext.instance,
                              debugMessenger); // this depends on whether we are running in debug or not

    err = createSurface(_NanoContext.instance,
                        window.getGLFWwindow(),
                        _NanoContext.surface);

    err = pickPhysicalDevice(_NanoContext.instance,
                             _NanoContext.surface,
                             _NanoContext.queueIndices,
                             _NanoContext.physicalDevice); // physical device is not created but picked based on scores dictated by the number of supported features

    err = createLogicalDevice(_NanoContext.physicalDevice,
                              _NanoContext.queueIndices,
                              _NanoContext.presentQueue,
                              _NanoContext.graphicsQueue,
                              _NanoContext.device); // Logical device *is* created and therefore has to be destroyed

    err = createSwapChain(_NanoContext.physicalDevice,
                          _NanoContext.device,
                          window.getGLFWwindow(),
                          _NanoContext.surface,
                          _NanoContext.swapchain,
                          _NanoContext.swapchainInfo,
                          _NanoContext.swapchainImages);

    err = createSCImageViews(_NanoContext.device,
                             _NanoContext.swapchainInfo,
                             _NanoContext.swapchainImages,
                             _NanoContext.swapchainImageViews);

    err = createRenderPass(_NanoContext.device,
                           _NanoContext.swapchainInfo,
                           _NanoContext.renderpass);

    NanoGraphicsPipeline graphicsPipeline{};
    err = createGraphicsPipeline(_NanoContext.device,
                                 _NanoContext.swapchainInfo,
                                 _NanoContext.renderpass,
                                 graphicsPipeline);

    _NanoContext.AddGraphicsPipeline(graphicsPipeline);

    err = createFramebuffer(_NanoContext.device,
                            _NanoContext.swapchainInfo,
                            _NanoContext.renderpass,
                            _NanoContext.swapchainImageViews,
                            _NanoContext.swapChainFramebuffers);

    err = createCommandPool(_NanoContext.device,
                            _NanoContext.queueIndices,
                            _NanoContext.commandPool);

    err = createCommandBuffer(_NanoContext.device,
                              _NanoContext.commandPool,
                              _NanoContext.commandBuffer);

    return err;
}

ERR NanoGraphics::DrawFrame(){
    ERR err = ERR::OK;
    return err;
}
