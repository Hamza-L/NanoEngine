#include "NanoShader.hpp"
#include <windows.h>
#include <thread>

VOID RunGLSLCompiler(LPCTSTR lpApplicationName, char const* argv[], const char* shaderName)
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
  LOG_MSG(ERRLevel::INFO, "finished compiling: %s", shaderName);

  // Close process and thread handles.
  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );
}

ERR NanoShader::Init(const char* shaderCodeFile){
    ERR err = ERR::OK;
    m_fileFullPath = shaderCodeFile;
    return err;
}

ERR NanoShader::Compile(){
    ERR err = ERR::OK;

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
    RunGLSLCompiler(executable, argv, m_fileFullPath.substr(startIndx).c_str());

    return err;
}
