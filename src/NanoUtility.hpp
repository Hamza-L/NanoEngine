#ifndef NANOUTILITY_H_
#define NANOUTILITY_H_

#include <cstdio>
#include <NanoError.hpp>

#define PRINTFLUSH(content, ...) printf(content, __VA_ARGS__); fflush(stdout)

#define VK_CHECK_THROW(func, msg) if(func != VK_SUCCESS){throw std::runtime_error(msg);}

namespace Utility{

    inline size_t SizeOf(const char* const* array){
        size_t size = 0;
        while(array[size]){
            size++;
        }
        return size;
    }

}

#endif // NANOUTILITY_H_
