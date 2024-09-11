#include "NanoWindow.hpp"
#include "GLFW/glfw3.h"
#include "NanoError.hpp"
#include "NanoConfig.hpp"

ERR NanoWindow::Init(const int32_t width, const int32_t height, bool forceReInit){
    ERR err = ERR::OK;
    if(m_isInit && !forceReInit)
        return ERR::NOT_INITIALIZED;

    CleanUp();
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(width, height, APP_NAME, nullptr, nullptr);
    if(m_window){
        m_isInit = true;
    } else {
        m_isInit = false;
        err = NOT_INITIALIZED;
    }
    return err;
}

ERR NanoWindow::Init(){
    return Init(WINDOW_WIDTH, WINDOW_HEIGHT, false);
}

void NanoWindow::PollEvents(){
    glfwPollEvents();
}

bool NanoWindow::ShouldWindowClose(){
    if(!m_isInit){
        return true;
    }

    return glfwWindowShouldClose(m_window);
}

ERR NanoWindow::CleanUp(){
    ERR err = ERR::OK;
    if(!m_isInit){
        return ERR::NOT_INITIALIZED;
    }
    glfwDestroyWindow(m_window);
    glfwTerminate();
    return err;
}
