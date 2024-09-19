#ifndef NANOCONFIG_H_
#define NANOCONFIG_H_

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Config {
constexpr uint16_t WINDOW_WIDTH = 800;
constexpr uint16_t WINDOW_HEIGHT = 600;
constexpr const char *APP_NAME = "NanoApplication";
constexpr const char *ENGINE_NAME = "NanoEngine";

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr const char *desiredValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
    NULL // to allow for while loops without crash
};

constexpr const char *deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    NULL // to allow for while loops without crash
};
} // namespace Config

#endif // NANOCONFIG_H_
