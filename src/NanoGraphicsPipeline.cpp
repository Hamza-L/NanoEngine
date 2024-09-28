#include "NanoGraphicsPipeline.hpp"

void NanoGraphicsPipeline::Init(VkDevice& device){
    _device = device;
}

void NanoGraphicsPipeline::AddVertShader(const std::string& vertFileName){
    m_vertShader = {};
    m_vertShader.Init(_device, vertFileName);
    m_vertShader.Compile();
}

void NanoGraphicsPipeline::AddFragShader(const std::string& fragFileName){
    m_fragShader = {};
    m_fragShader.Init(_device, fragFileName);
    m_fragShader.Compile();
}

int NanoGraphicsPipeline::Compile(bool forceReCompile){
    int res = 0;
    return res;
}

void NanoGraphicsPipeline::CleanUp(){
    m_fragShader.CleanUp();
    m_vertShader.CleanUp();
}
