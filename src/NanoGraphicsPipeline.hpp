#ifndef NANOGRAPHICSPIPELINE_H_
#define NANOGRAPHICSPIPELINE_H_

#include "NanoShader.hpp"

class NanoGraphicsPipeline{
    public:
        void Init(VkDevice& device);
        void AddVertShader(const std::string& vertShaderFile);
        void AddFragShader(const std::string& fragShaderFile);
        int Compile(bool forceReCompile = false);
        void CleanUp();
    private:
        VkDevice _device = {};
        NanoShader m_vertShader = {};
        VkShaderModule m_vertShaderModule{};
        NanoShader m_fragShader = {};
        VkShaderModule m_fragShaderModule{};
};
#endif // NANOGRAPHICSPIPELINE_H_
