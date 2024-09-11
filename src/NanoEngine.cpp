#include "NanoEngine.hpp"

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
