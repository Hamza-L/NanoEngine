#include "NanoGraphics.hpp"
#include "NanoConfig.hpp"
#include "NanoError.hpp"
#include "NanoUtility.hpp"
#include "NanoLogger.hpp"
#include "vulkan/vulkan_core.h"
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <map>

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
};

struct NanoVKContext{
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    struct QueueFamilyIndices queueIndices;
} _NanoContext;

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
        messageLevel = ERRLevel::ERROR;
        break;
    default:
        messageLevel = ERRLevel::INFO;
        break;
    }

    LOG_MSG(messageLevel, "%s", pCallbackData->pMessage);
    fprintf(stderr, "Objects involved: \n");
    for(int i = 0; i < pCallbackData->objectCount; i++){
        fprintf(stderr, "\t%s\n", pCallbackData->pObjects[i].pObjectName);
    }
    fprintf(stderr, "\n");

    if(messageLevel == ERRLevel::FATAL)
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

ERR NanoGraphics::CleanUp(){
    ERR err = ERR::OK;
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(_NanoContext.instance, _NanoContext.debugMessenger, nullptr);
    }

    vkDestroyInstance(_NanoContext.instance, nullptr);

    return err;
}

static bool checkValidationLayerSupport(const char* const* validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    int indx = 0;
    while(validationLayers[indx]) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
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

static std::vector<const char*> getRequiredInstanceExtensions(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // additional instance extension we may want to add
    std::vector<const char*> instanceExtensions;
    for(uint32_t i = 0; i < glfwExtensionCount; i++) {
        instanceExtensions.emplace_back(glfwExtensions[i]);
    }
#if MACOS
    instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    if (enableValidationLayers) {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return instanceExtensions;
    // ----------------------------------------
}

static std::vector<VkExtensionProperties>& getSupportedInstanceExtensions(std::vector<VkExtensionProperties>& extensions){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    extensions.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

#ifdef _DEBUG
    std::cout << "available extensions:\n";
    for (const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
    std::cout.flush();
#endif

    return extensions;
}

static ERR createInstance(VkInstance& instance, const char* applicationName, const char* engineName) {
    ERR err = ERR::OK;

    if (enableValidationLayers && !checkValidationLayerSupport(desiredValidationLayers)) {
        LOG_MSG( ERRLevel::FATAL, "Number of Desired Layers %zu\n", Utility::SizeOf(desiredValidationLayers));
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

    std::vector<const char*> instanceExtensions = getRequiredInstanceExtensions();
    createInfo.enabledExtensionCount = (uint32_t) instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(Utility::SizeOf(desiredValidationLayers));
        createInfo.ppEnabledLayerNames = desiredValidationLayers;

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

static ERR setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger){
    ERR err = ERR::OK;

    if(!enableValidationLayers){
        return ERR::OK;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    VK_CHECK_THROW(CreateDebugUtilsMessengerEXT(_NanoContext.instance, &createInfo, nullptr, &_NanoContext.debugMessenger), "Failed to create Debug Messenger\n");
    return ERR::OK;//this is never reached if we use try/catch.
}

int rateDeviceSuitability(VkPhysicalDevice device) {
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

    return score;
}

static ERR pickPhysicalDevice(VkInstance& instance, VkPhysicalDevice& physicalDevice){
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

    for (const auto& device : devices) {
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    int bestScore = 0;
    for(auto & candidate : candidates){
        if (candidate.first > 0 && candidate.first > bestScore) {
            bestScore = candidate.first;
            physicalDevice = candidates.rbegin()->second;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE){
        err = ERR::NOT_FOUND;
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    return ERR::OK;//this is never reached if we use try/catch.
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // Logic to find queue family indices to populate struct with
    return indices;
}

ERR NanoGraphics::Init(){
    ERR err = ERR::OK;
    err = createInstance(_NanoContext.instance, APP_NAME, ENGINE_NAME);
    err = setupDebugMessenger(_NanoContext.instance, _NanoContext.debugMessenger);
    err = pickPhysicalDevice(_NanoContext.instance, _NanoContext.physicalDevice);
    return err;
}
