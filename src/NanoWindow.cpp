#include "NanoWindow.hpp"

#include "NanoLogger.hpp"
#include "NanoConfig.hpp"

struct NanoWindowContext{
    GLFWwindow* window;
}_NanoWindow;

ERR NanoWindow::Init(const int32_t width, const int32_t height, bool forceReInit){
    ERR err = ERR::OK;
    if(m_isInit && !forceReInit)
        return ERR::NOT_INITIALIZED;

    CleanUp();
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _NanoWindow.window = glfwCreateWindow(width, height, Config::APP_NAME, nullptr, nullptr);
    if(_NanoWindow.window){
        m_isInit = true;
    } else {
        m_isInit = false;
        err = ERR::NOT_INITIALIZED;
    }
    return err;
}

ERR NanoWindow::Init(){
    return Init(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, false);
}

void NanoWindow::PollEvents(){
    glfwPollEvents();
}

bool NanoWindow::ShouldWindowClose(){
    if(!m_isInit){
        return true;
    }

    return glfwWindowShouldClose(_NanoWindow.window);
}

GLFWwindow* NanoWindow::getGLFWwindow(){
    return m_isInit ? _NanoWindow.window : nullptr;
}

ERR NanoWindow::CleanUp(){
    ERR err = ERR::OK;
    if(!m_isInit){
        return ERR::NOT_INITIALIZED;
    }
    glfwDestroyWindow(_NanoWindow.window);
    glfwTerminate();
    return err;
}
