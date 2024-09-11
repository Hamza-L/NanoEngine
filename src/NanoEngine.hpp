#ifndef NANOENGINE_H_
#define NANOENGINE_H_

#include "NanoGraphics.hpp"
#include "NanoWindow.hpp"
#include <cstdint>

class NanoEngine {
  public:
    ERR Init();
    ERR Run();

  private:
    ERR MainLoop();
    NanoGraphics m_NGraphics;
    NanoWindow m_NWindow;
};

#endif // NANOENGINE_H_
