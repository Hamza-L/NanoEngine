#include "NanoShader.hpp"
#include "NanoError.hpp"
#include "NanoLogger.hpp"
#include <windows.h>
#include <fstream>

void NanoShader::CleanUp(){
    vkDestroyShaderModule(_device, m_shaderModule, nullptr);
}

static VkShaderModule CreateShaderModule(VkDevice& device, NanoShader& shader) {
  ERR err = ERR::OK;
  VkShaderModule shaderModule = {};
  if(!shader.IsCompiled()){
      LOG_MSG(ERRLevel::WARNING, "Attempting to create a shader module from uncompiled shader");
      return shaderModule; // failed the re-attempt at compiling
  }

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shader.GetByteCode().size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.GetByteCode().data());

  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

int RunGLSLCompiler(LPCTSTR lpApplicationName, char const* argv[], const char* shaderName)
{
   // additional information
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   // set the size of the structures
   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

  // start the program up
  bool test = CreateProcess( lpApplicationName,   // the path
    const_cast<char*>(argv[0]),           // Command line
    NULL,           // Process handle not inheritable
    NULL,           // Thread handle not inheritable
    FALSE,          // Set handle inheritance to FALSE
    0,              // No creation flags
    NULL,           // Use parent's environment block
    NULL,           // Use parent's starting directory
    &si,            // Pointer to STARTUPINFO structure
    &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );

  // Wait until child process exits.
  LOG_MSG(ERRLevel::INFO, "compiling : %s", shaderName);
  WaitForSingleObject( pi.hProcess, INFINITE );

  DWORD exit_code;
  GetExitCodeProcess(pi.hProcess, &exit_code);

  int compilerExitCode = (int)exit_code;
  LOG_MSG(ERRLevel::INFO, "finished compiling: %s\t with exit code: %d", shaderName, compilerExitCode);

  // Close process and thread handles.
  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );

  return compilerExitCode;
}

static std::vector<char> ReadBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void NanoShader::Init(VkDevice& device, const std::string& shaderCodeFile){
    m_fileFullPath = shaderCodeFile;
    _device = device;
}

int NanoShader::Compile(bool forceCompile){
    int exitCode = 1;
    std::string outputFile = "./src/shader/";

    if(m_fileFullPath.find(".vert") != std::string::npos){
        outputFile.append("vert");
    } else if (m_fileFullPath.find(".frag") != std::string::npos){
        outputFile.append("frag");
    } else if (m_fileFullPath.find(".comp") != std::string::npos){
        outputFile.append("comp");
    }
    outputFile.append("_");

    int startIndx = m_fileFullPath.find_last_of("/") + 1;
    int endIndx = m_fileFullPath.find_last_of(".");
    std::string fileName = m_fileFullPath.substr(startIndx, endIndx-startIndx);

    outputFile.append(fileName.c_str());
    outputFile.append(".spv");

    std::string cmdArgument = " ";
    cmdArgument.append(m_fileFullPath);
    cmdArgument.append(" -o ");
    cmdArgument.append(outputFile);

    char const *argv[] = {cmdArgument.c_str()};
    LPCTSTR executable = "./external/VULKAN/win/glslc.exe";
    exitCode = RunGLSLCompiler(executable, argv, m_fileFullPath.substr(startIndx).c_str());

    if(!exitCode){
      LOG_MSG(ERRLevel::INFO, "Successfully compiled")
      m_isCompiled = true;
      m_rawShaderCode = ReadBinaryFile(outputFile);
      m_shaderModule = CreateShaderModule(_device, *this);
    } else {
      m_isCompiled = false;
    }

    return exitCode;
}
