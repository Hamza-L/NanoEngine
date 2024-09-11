#include "NanoGraphics.hpp"
#include "NanoConfig.hpp"
#include "vulkan/vulkan_core.h"

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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> instanceExtensions;
    for(uint32_t i = 0; i < glfwExtensionCount; i++) {
        instanceExtensions.emplace_back(glfwExtensions[i]);
    }
#if MACOS
    instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    std::vector<VkExtensionProperties> test = {};
    _getSupportedInstanceExtensions(test);

    createInfo.enabledExtensionCount = (uint32_t) instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    createInfo.enabledLayerCount = 0;

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
