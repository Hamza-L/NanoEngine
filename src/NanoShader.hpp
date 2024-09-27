#ifndef NANOSHADER_H_
#define NANOSHADER_H_

#include "vulkan/vulkan_core.h"

#include "NanoUtility.hpp"
#include "NanoLogger.hpp"
#include "NanoError.hpp"
#include "NanoConfig.hpp"

class NanoShader{
    public:
        ERR Init(const char* shaderCodeFile);
        ERR Compile();
    private:
        std::string m_fileFullPath;
        std::string m_rawShaderCode;
};

#endif // NANOSHADER_H_
