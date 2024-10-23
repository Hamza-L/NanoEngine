#ifndef NANOGRAPHICSPIPELINE_H_
#define NANOGRAPHICSPIPELINE_H_

#include "NanoShader.hpp"
#include "vulkan/vulkan_core.h"

class NanoGraphicsPipeline{
    public:
        void Init(VkDevice& device, const VkExtent2D& extent);
        void AddVertShader(const std::string& vertShaderFile);
        void AddFragShader(const std::string& fragShaderFile);
        void AddRenderPass(const VkRenderPass& renderpass);
        void ConfigureViewport(const VkExtent2D& extent);
        ERR Compile(bool forceReCompile = false);
        void CleanUp();

        VkPipeline& GetPipeline(){return m_pipeline;}
        VkRenderPass& GetRenderPass(){return _renderpass;}
        VkExtent2D& GetExtent(){return m_extent;}
    private:
        VkDevice _device = {};
        VkRenderPass _renderpass = {};
        VkExtent2D m_extent = {};
        NanoShader m_vertShader = {};
        NanoShader m_fragShader = {};
        VkPipelineLayout m_pipelineLayout = {};
        VkPipeline m_pipeline = {};
};
#endif // NANOGRAPHICSPIPELINE_H_
