#ifndef NANOCONFIG_H_
#define NANOCONFIG_H_

#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace Config {
constexpr uint16_t WINDOW_WIDTH = 800;
constexpr uint16_t WINDOW_HEIGHT = 600;
constexpr const char *APP_NAME = "NanoApplication";
constexpr const char *ENGINE_NAME = "NanoEngine";
constexpr int MAX_FRAMES_IN_FLIGHT = 2;


#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr const char *desiredValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
    NULL // to allow for while loops without crash
};

constexpr const char *desiredDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
    NULL // to allow for while loops without crash
};

constexpr const char *desiredInstanceExtensions[] = {
#ifdef __APPLE__
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    "VK_KHR_get_physical_device_properties2",
#endif
    NULL // to allow for while loops without crash
};
} // namespace Config

#endif // NANOCONFIG_H_
