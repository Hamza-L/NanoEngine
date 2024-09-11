#ifndef NANOWINDOW_H_
#define NANOWINDOW_H_

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <cstdint>

class NanoWindow{
public:
        NanoWindow(const int32_t width, const int32_t height, const char* windowName);
        NanoWindow(const NanoWindow& other);
        NanoWindow(const NanoWindow&& other);
        NanoWindow& operator=(const NanoWindow& other);
        ~NanoWindow();
        void Init(const int32_t width, const int32_t height, const char* windowName, bool forceReInit);
        bool ShouldWindowClose();
        void CleanUp();
private:
        bool m_isInit = false;
        GLFWwindow* m_window = nullptr;

};

#endif // NANOWINDOW_H_
