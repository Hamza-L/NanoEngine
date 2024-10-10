#ifndef NANOGRAPHICSPIPELINE_H_
#define NANOGRAPHICSPIPELINE_H_

#include "NanoShader.hpp"
#include "NanoConfig.hpp"

class NanoGraphicsPipeline{
    public:
        void Init(VkDevice& device, const VkExtent2D& extent);
        void AddVertShader(const std::string& vertShaderFile);
        void AddFragShader(const std::string& fragShaderFile);
        void ConfigureViewport(const VkExtent2D& extent);
        int Compile(bool forceReCompile = false);
        void CleanUp();
    private:
        VkDevice _device = {};
        VkExtent2D m_extent = {};
        NanoShader m_vertShader = {};
        NanoShader m_fragShader = {};
};
#endif // NANOGRAPHICSPIPELINE_H_
