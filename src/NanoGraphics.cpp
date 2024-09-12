#include "NanoGraphics.hpp"
#include "NanoConfig.hpp"
#include "NanoUtility.hpp"
#include "NanoLogger.hpp"
#include "vulkan/vulkan_core.h"
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

struct NanoVKContext{
    VkInstance instance{};
} _NanoContext;

ERR NanoGraphics::CleanUp(){
    ERR err = ERR::OK;

    vkDestroyInstance(_NanoContext.instance, nullptr);

    return err;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
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

std::vector<const char*> _getRequiredInstanceExtensions(){
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

std::vector<VkExtensionProperties>& _getSupportedInstanceExtensions(std::vector<VkExtensionProperties>& extensions){
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

ERR _createInstance(VkInstance& instance, const char* applicationName, const char* engineName) {
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

    std::vector<const char*> instanceExtensions = _getRequiredInstanceExtensions();

    // std::vector<VkExtensionProperties> test = {};
    // _getSupportedInstanceExtensions(test);

    createInfo.enabledExtensionCount = (uint32_t) instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    if (enableValidationLayers) {
        LOG_MSG( ERRLevel::INFO, "Number of Desired Layers %zu\n", Utility::SizeOf(desiredValidationLayers));
        createInfo.enabledLayerCount = static_cast<uint32_t>(Utility::SizeOf(desiredValidationLayers));
        createInfo.ppEnabledLayerNames = desiredValidationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        return ERR::UNDEFINED;
    }
    return err;
}

ERR NanoGraphics::Init(){
    ERR err = ERR::OK;
    err = _createInstance(_NanoContext.instance, APP_NAME, ENGINE_NAME);
    return err;
}
