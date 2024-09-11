#include "NanoEngine.hpp"

NanoEngine::~NanoEngine(){
    CleanUp();
}

ERR NanoEngine::CleanUp(){
    ERR err = ERR::OK;
    m_NGraphics.CleanUp();
    m_NWindow.CleanUp();
    return err;
}

ERR NanoEngine::Init(){
    ERR err = ERR::OK;
    err = m_NWindow.Init();
    err = m_NGraphics.Init();
    return err;
}

ERR NanoEngine::Run(){
    ERR err = ERR::OK;
    while(!m_NWindow.ShouldWindowClose()){
        m_NWindow.PollEvents();

        MainLoop();

    }
    return err;
}

ERR NanoEngine::MainLoop(){
    ERR err = ERR::OK;

    return err;
}
