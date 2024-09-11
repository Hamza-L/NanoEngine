#include "NanoWindow.hpp"
#include "GLFW/glfw3.h"

NanoWindow::NanoWindow(const int32_t width, const int32_t height, const char* windowName){

}

NanoWindow::NanoWindow(const NanoWindow& other){

}

NanoWindow::NanoWindow(const NanoWindow&& other){

}

NanoWindow& NanoWindow::operator=(const NanoWindow& other){
    if (this != &other) {
        // generate a deep copy of resources
    }
    return *this;
}

NanoWindow::~NanoWindow(){
    if(!m_isInit){
        return;
    }
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void NanoWindow::Init(const int32_t width, const int32_t height, const char* windowName, bool forceReInit){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
    if(m_window){
        m_isInit = true;
    } else {
        m_isInit = false;
    }
}

bool NanoWindow::ShouldWindowClose(){
    if(!m_isInit){
        return true;
    }

    return glfwWindowShouldClose(m_window);
}
