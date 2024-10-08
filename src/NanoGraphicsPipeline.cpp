#include "NanoGraphicsPipeline.hpp"
#include "vulkan/vulkan_core.h"

//TODO ; Match the init design of the NanoGraphics class
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

void NanoGraphicsPipeline::ConfigureViewport(const VkExtent2D& extent){

}

int NanoGraphicsPipeline::Compile(bool forceReCompile){
    int res = 0;

    VkPipelineShaderStageCreateInfo vertexShaderStageCI = {};
    vertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCI.module = m_vertShader.GetShaderModule();
    vertexShaderStageCI.pName = "main";
    // VkSpecializationInfo specializationInfo = {};
    vertexShaderStageCI.pSpecializationInfo = nullptr; //Can specify different values for constant used in this shader. allows better optimization at shader creation stage

    VkPipelineShaderStageCreateInfo fragmentShaderStageCI = {};
    fragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
    fragmentShaderStageCI.module = m_vertShader.GetShaderModule();
    fragmentShaderStageCI.pName = "main";
    // VkSpecializationInfo specializationInfo = {};
    fragmentShaderStageCI.pSpecializationInfo = nullptr; //Can specify different values for constant used in this shader. allows better optimization at shader creation stage

    VkPipelineShaderStageCreateInfo shaderStagesCI[] = {vertexShaderStageCI, fragmentShaderStageCI};

    VkPipelineVertexInputStateCreateInfo vertexInputCI = {};
    vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCI.vertexBindingDescriptionCount = 0;
    vertexInputCI.pVertexBindingDescriptions = nullptr;
    vertexInputCI.vertexAttributeDescriptionCount = 0;
    vertexInputCI.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = {};
    inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCI.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;

    return res;
}

void NanoGraphicsPipeline::CleanUp(){
    m_fragShader.CleanUp();
    m_vertShader.CleanUp();
}
