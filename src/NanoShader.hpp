#ifndef NANOSHADER_H_
#define NANOSHADER_H_

#include "NanoError.hpp"

#include "vulkan/vulkan_core.h"
#include <string>
#include <vector>

class NanoShader{
    public:
        void Init(VkDevice& device, const std::string& shaderCodeFile);
        int Compile(bool forceCompile = false);
        void CleanUp();
        bool IsCompiled(){return m_isCompiled;};
        std::vector<char>& GetByteCode(){return m_rawShaderCode;};
        VkShaderModule& GetShaderModule(){return m_shaderModule;};
    private:
        VkDevice _device;
        std::string m_fileFullPath{};
        std::vector<char> m_rawShaderCode{};
        bool m_isCompiled = false;

        VkShaderModule m_shaderModule{};
};

#endif // NANOSHADER_H_
