#ifndef NANOCONFIG_H_
#define NANOCONFIG_H_

#include <cstdint>
#include <vector>

constexpr uint16_t WINDOW_WIDTH = 800;
constexpr uint16_t WINDOW_HEIGHT = 600;
constexpr const char* APP_NAME = "NanoApplication";
constexpr const char* ENGINE_NAME = "NanoEngine";

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr const char* desiredValidationLayers[] = {
"test",
"test2",
NULL //to allow for while loops without crash
};


#endif // NANOCONFIG_H_
