#ifndef NANOENGINE_H_
#define NANOENGINE_H_

#include "NanoGraphics.hpp"
#include "NanoWindow.hpp"
#include <cstdint>

class NanoEngine {
  public:
    NanoEngine() = default;
    ~NanoEngine();
    NanoEngine(const NanoEngine &other) = default;
    NanoEngine(NanoEngine &&other) = default;
    NanoEngine &operator=(const NanoEngine &other) = default;
    ERR Init();
    ERR Run();
    ERR CleanUp();

  private:
    ERR MainLoop();
    NanoGraphics m_NanoGraphics;
    NanoWindow m_NanoWindow;
};

#endif // NANOENGINE_H_
