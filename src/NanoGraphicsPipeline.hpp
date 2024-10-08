#ifndef NANOGRAPHICSPIPELINE_H_
#define NANOGRAPHICSPIPELINE_H_

#include "NanoShader.hpp"

class NanoGraphicsPipeline{
    public:
        void Init(VkDevice& device);
        void AddVertShader(const std::string& vertShaderFile);
        void AddFragShader(const std::string& fragShaderFile);
        void ConfigureViewport(const VkExtent2D& extent);
        int Compile(bool forceReCompile = false);
        void CleanUp();
    private:
        VkDevice _device = {};
        NanoShader m_vertShader = {};
        NanoShader m_fragShader = {};
};
#endif // NANOGRAPHICSPIPELINE_H_
