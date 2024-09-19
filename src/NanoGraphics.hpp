#ifndef NANOGRAPHICS_H_
#define NANOGRAPHICS_H_

#include "NanoLogger.hpp"
#include "NanoWindow.hpp"

class NanoGraphics{
    public:
        ERR Init(NanoWindow& window);
        ERR CleanUp();
    private:
};

#endif // NANOGRAPHICS_H_
