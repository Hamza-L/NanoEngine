#include "NanoGraphics.hpp"
#include "NanoConfig.hpp"

void _createInstance(const char* applicationName, const char* engineName) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
}

ERR NanoGraphics::Init(){
    ERR err = ERR::OK;
    _createInstance(APP_NAME, ENGINE_NAME);
    return err;
}
