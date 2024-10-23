#include "NanoGraphicsPipeline.hpp"
#include "vulkan/vulkan_core.h"

//TODO ; Match the init design of the NanoGraphics class
void NanoGraphicsPipeline::Init(VkDevice& device, const VkExtent2D& extent){
    _device = device;
    m_extent = extent;
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

void NanoGraphicsPipeline::AddRenderPass(const VkRenderPass& renderpass){
    _renderpass = renderpass;
}

void NanoGraphicsPipeline::ConfigureViewport(const VkExtent2D& extent){
    m_extent = extent;
}

ERR NanoGraphicsPipeline::Compile(bool forceReCompile){
    ERR err = ERR::OK;

    if(!m_vertShader.IsCompiled()){
        ASSERT(m_vertShader.IsCompiled(), "graphics pipeline's vertex shader was not compiled\n");
        return ERR::NOT_INITIALIZED;
    }

    if(!m_fragShader.IsCompiled()){
        ASSERT(m_fragShader.IsCompiled(), "graphics pipeline's fragment shader was not compiled\n");
        return ERR::NOT_INITIALIZED;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Shaders ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkPipelineShaderStageCreateInfo vertexShaderStage = {};
    vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStage.module = m_vertShader.GetShaderModule();
    vertexShaderStage.pName = "main";
    // VkSpecializationInfo specializationInfo = {};
    vertexShaderStage.pSpecializationInfo = nullptr; //Can specify different values for constant used in this shader. allows better optimization at shader creation stage

    VkPipelineShaderStageCreateInfo fragmentShaderStage = {};
    fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage.module = m_fragShader.GetShaderModule();
    fragmentShaderStage.pName = "main";
    // VkSpecializationInfo specializationInfo = {};
    fragmentShaderStage.pSpecializationInfo = nullptr; //Can specify different values for constant used in this shader. allows better optimization at shader creation stage

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStage, fragmentShaderStage};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Input assembly ////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Viewport & Scissor ////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = m_extent.width;
    viewport.height = m_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    //viewportState.pViewports = &viewport; //if we want the viewport to be immutable and not dynamic, we would create it and add it here
    viewportState.scissorCount = 1;
    //viewportState.pScissors = &scissor; //if we want the scissor to be immutable and not dynamic, we would create it and add it here

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Rasterizer ////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.
    // This is useful in some special cases like shadow maps. Using this requires enabling a GPU feature. Keep it false for now
    rasterizer.depthClampEnable = VK_FALSE;
    // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // using anything other than fill requires enabling a gpu feature (this can allow us to set line width and point size)
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE; // sometimes used for shadow mapping
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    rasterizer.lineWidth = 1.0f;


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Multi-sampling ////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // leave multi-sampling (for AA) disabled for now
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{}; // optional.

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Color blending ////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Pipeline Layout ///////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = _renderpass;
    pipelineInfo.subpass = 0; // subpass to use
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        err = ERR::INVALID;
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return err;
}

void NanoGraphicsPipeline::CleanUp(){
    vkDestroyPipeline(_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, m_pipelineLayout, nullptr);
    m_fragShader.CleanUp();
    m_vertShader.CleanUp();
}
