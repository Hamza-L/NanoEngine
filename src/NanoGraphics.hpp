#ifndef NANOGRAPHICS_H_
#define NANOGRAPHICS_H_

#include <vulkan/vulkan_core.h>
#include "NanoError.hpp"

class NanoGraphics{
    public:
        ERR Init();
        ERR CleanUp();
    private:
};

#endif // NANOGRAPHICS_H_
