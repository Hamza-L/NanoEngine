#include "NanoEngine.hpp"

NanoEngine::~NanoEngine(){
    CleanUp();
}

ERR NanoEngine::CleanUp(){
    ERR err = ERR::OK;
    m_NanoGraphics.CleanUp();
    m_NanoWindow.CleanUp();
    return err;
}

ERR NanoEngine::Init(){
    ERR err = ERR::OK;
    err = m_NanoWindow.Init();
    err = m_NanoGraphics.Init(m_NanoWindow);
    return err;
}

ERR NanoEngine::Run(){
    ERR err = ERR::OK;
    while(!m_NanoWindow.ShouldWindowClose()){
        m_NanoWindow.PollEvents();

        MainLoop();

    }
    return err;
}

ERR NanoEngine::MainLoop(){
    ERR err = ERR::OK;

    return err;
}
